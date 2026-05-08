/*
 * Copyright 2021 The Chromium OS Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdbool.h>
#include "smf.h"
#include "gw_log.h"
#define SMF_TAG "SMF"
/**
 * @brief 私有结构体（仅限本文件使用），用于跟踪状态机上下文。
 *        该结构体不直接使用，而是用于强制转换 smf_ctx 结构体中的 "internal" 成员。
 */
struct internal_ctx {
	bool new_state: 1;
	bool terminate: 1;
	bool is_exit: 1;
	bool handled: 1;
};

#ifdef CONFIG_SMF_INSTRUMENTATION
static inline void invoke_action_hook(struct smf_ctx *ctx, const struct smf_state *state,
				      enum smf_action_type type)
{
	if (ctx->hooks && ctx->hooks->on_action) {
		ctx->hooks->on_action(ctx, state, type);
	}
}

static inline void invoke_transition_hook(struct smf_ctx *ctx, const struct smf_state *source,
					  const struct smf_state *dest)
{
	if (ctx->hooks && ctx->hooks->on_transition) {
		ctx->hooks->on_transition(ctx, source, dest);
	}
}

static inline void invoke_error_hook(struct smf_ctx *ctx, int code)
{
	if (ctx->hooks && ctx->hooks->on_error) {
		ctx->hooks->on_error(ctx, code);
	}
}

#define INVOKE_ACTION_HOOK(ctx, state, type) invoke_action_hook(ctx, state, type)

#define INVOKE_TRANSITION_HOOK(ctx, source, dest) invoke_transition_hook(ctx, source, dest)

#define INVOKE_ERROR_HOOK(ctx, code) invoke_error_hook(ctx, code)

#else /* CONFIG_SMF_INSTRUMENTATION */

#define INVOKE_ACTION_HOOK(ctx, state, type)                                                       \
	do {                                                                                       \
	} while (false)

#define INVOKE_TRANSITION_HOOK(ctx, source, dest)                                                  \
	do {                                                                                       \
	} while (false)

#define INVOKE_ERROR_HOOK(ctx, code)                                                               \
	do {                                                                                       \
	} while (false)

#endif /* CONFIG_SMF_INSTRUMENTATION */

#ifdef CONFIG_SMF_ANCESTOR_SUPPORT
static bool is_descendant_of(const struct smf_state *test_state,
			     const struct smf_state *target_state)
{
	for (const struct smf_state *state = test_state; state != NULL; state = state->parent) {
		if (target_state == state) {
			return true;
		}
	}

	return false;
}

static const struct smf_state *get_child_of(const struct smf_state *states,
					    const struct smf_state *parent)
{
	const struct smf_state *state = states;

	while (state != NULL) {
		if (state->parent == parent) {
			return state;
		}

		state = state->parent;
	}

	return NULL;
}

/**
 * @brief 查找两个状态的最小公共祖先 (LCA)，
 *	  这两个状态彼此不是祖先关系。
 *
 * @param source 转换源状态
 * @param dest 转换目标状态
 * @return LCA 状态，如果状态没有 LCA 则返回 NULL。
 */
static const struct smf_state *get_lca_of(const struct smf_state *source,
					  const struct smf_state *dest)
{
	for (const struct smf_state *ancestor = source->parent; ancestor != NULL;
	     ancestor = ancestor->parent) {
		/* 第一个公共祖先 */
		if (is_descendant_of(dest, ancestor)) {
			return ancestor;
		}
	}

	return NULL;
}

/**
 * @brief 执行从顶层状态的直接子状态到新状态的所有入口动作
 *
 * @param ctx 状态机上下文
 * @param new_state 我们要转换到的状态
 * @param topmost 我们进入的来源状态。其入口动作不会被执行
 * @return 如果状态机应终止则返回 true，否则返回 false
 */
static bool smf_execute_all_entry_actions(struct smf_ctx *const ctx,
					  const struct smf_state *new_state,
					  const struct smf_state *topmost)
{
	struct internal_ctx *const internal = (void *)&ctx->internal;

	if (new_state == topmost) {
		/* 没有子状态，因此不执行任何操作 */
		return false;
	}

	for (const struct smf_state *to_execute = get_child_of(new_state, topmost);
	     to_execute != NULL && to_execute != new_state;
	     to_execute = get_child_of(new_state, to_execute)) {
		/* 跟踪正在执行的入口动作，以防它调用 smf_set_state() */
		ctx->executing = to_execute;
		/* 执行每个入口动作，除了顶层状态的动作 */
		if (to_execute->entry) {
			INVOKE_ACTION_HOOK(ctx, to_execute, SMF_ACTION_ENTRY);
			to_execute->entry(ctx);

			/* 如果设置了终止标志，无需继续 */
			if (internal->terminate) {
				ctx->executing = ctx->current;
				return true;
			}
		}
	}

	/* 并执行新状态的入口动作 */
	ctx->executing = new_state;
	if (new_state->entry) {
		INVOKE_ACTION_HOOK(ctx, new_state, SMF_ACTION_ENTRY);
		new_state->entry(ctx);

		/* 如果设置了终止标志，无需继续 */
		if (internal->terminate) {
			ctx->executing = ctx->current;
			return true;
		}
	}

	ctx->executing = ctx->current;

	return false;
}

/**
 * @brief 执行所有祖先的 run 动作
 *
 * @param ctx 状态机上下文
 * @param target 执行此目标状态祖先的 run 动作
 * @return 如果状态机应终止则返回 true，否则返回 false
 */
static bool smf_execute_ancestor_run_actions(struct smf_ctx *const ctx)
{
	struct internal_ctx *const internal = (void *)&ctx->internal;
	/* 以相反顺序执行所有 run 动作 */

	/* 如果当前状态已终止，则返回 */
	if (internal->terminate) {
		return true;
	}

	/* 子状态要么发生了转换，要么已处理事件。无论哪种情况，停止传播。 */
	if (internal->new_state || internal->handled) {
		return false;
	}

	/* 尝试运行父状态的 run 动作 */
	for (const struct smf_state *tmp_state = ctx->current->parent; tmp_state != NULL;
	     tmp_state = tmp_state->parent) {
		/* 跟踪当前位置，以防祖先调用 smf_set_state() */
		ctx->executing = tmp_state;
		/* 执行父状态的 run 动作 */
		if (tmp_state->run) {
			INVOKE_ACTION_HOOK(ctx, tmp_state, SMF_ACTION_RUN);
			enum smf_state_result rc = tmp_state->run(ctx);

			if (rc == SMF_EVENT_HANDLED) {
				internal->handled = true;
			}
			/* 如果设置了终止标志，无需继续 */
			if (internal->terminate) {
				ctx->executing = ctx->current;
				return true;
			}

			/* 此状态已处理完毕。停止传播。 */
			if (internal->new_state || internal->handled) {
				break;
			}
		}
	}

	/* 执行完所有 run 动作 */

	ctx->executing = ctx->current;

	return false;
}

/**
 * @brief 执行从 ctx->current 到 topmost 的直接子状态的所有退出动作
 *
 * @param ctx 状态机上下文
 * @param topmost 我们退出的目标状态。其退出动作不会被执行
 * @return 如果状态机应终止则返回 true，否则返回 false
 */
static bool smf_execute_all_exit_actions(struct smf_ctx *const ctx, const struct smf_state *topmost)
{
	struct internal_ctx *const internal = (void *)&ctx->internal;
	const struct smf_state *tmp_state = ctx->executing;

	for (const struct smf_state *to_execute = ctx->current;
	     to_execute != NULL && to_execute != topmost; to_execute = to_execute->parent) {
		if (to_execute->exit) {
			ctx->executing = to_execute;
			INVOKE_ACTION_HOOK(ctx, to_execute, SMF_ACTION_EXIT);
			to_execute->exit(ctx);

			/* 如果在退出动作中设置了终止标志，无需继续 */
			if (internal->terminate) {
				ctx->executing = tmp_state;
				return true;
			}
		}
	}

	ctx->executing = tmp_state;

	return false;
}
#endif /* CONFIG_SMF_ANCESTOR_SUPPORT */

/**
 * @brief 将状态机的内部状态重置为默认值。
 * 应在进入 smf_set_initial() 和 smf_set_state() 时调用。
 *
 * @param ctx 状态机上下文。
 */
static void smf_clear_internal_state(struct smf_ctx *const ctx)
{
	struct internal_ctx *const internal = (void *)&ctx->internal;

	internal->is_exit = false;
	internal->terminate = false;
	internal->handled = false;
	internal->new_state = false;
}

void smf_set_initial(struct smf_ctx *const ctx, const struct smf_state *init_state)
{
#ifdef CONFIG_SMF_INITIAL_TRANSITION
	/*
	 * 最终目标将是目标状态包含的最深层叶状态。
	 * 将其设置为真实目标。
	 */
	while (init_state->initial != NULL) {
		init_state = init_state->initial;
	}
#endif

	smf_clear_internal_state(ctx);
	ctx->current = init_state;
	ctx->previous = NULL;
	ctx->terminate_val = 0;

#ifdef CONFIG_SMF_INSTRUMENTATION
	ctx->hooks = NULL;
#endif /* CONFIG_SMF_INSTRUMENTATION */

#ifdef CONFIG_SMF_ANCESTOR_SUPPORT
	struct internal_ctx *const internal = (void *)&ctx->internal;

	ctx->executing = init_state;
	/* topmost 是 init_state 的根祖先，其 parent == NULL */
	const struct smf_state *topmost = get_child_of(init_state, NULL);

	/* 执行顶层状态的入口动作，因为 smf_execute_all_entry_actions() 不会执行它 */
	if (topmost->entry) {
		ctx->executing = topmost;
		INVOKE_ACTION_HOOK(ctx, topmost, SMF_ACTION_ENTRY);
		topmost->entry(ctx);
		ctx->executing = init_state;
		if (internal->terminate) {
			/* 如果设置了终止标志，无需继续 */
			return;
		}
	}

	if (smf_execute_all_entry_actions(ctx, init_state, topmost)) {
		/* 如果设置了终止标志，无需继续 */
		return;
	}
#else
	/* 如果存在入口动作，则执行 */
	if (init_state->entry) {
		INVOKE_ACTION_HOOK(ctx, init_state, SMF_ACTION_ENTRY);
		init_state->entry(ctx);
	}
#endif
}

void smf_set_state(struct smf_ctx *const ctx, const struct smf_state *new_state)
{
	struct internal_ctx *const internal = (void *)&ctx->internal;

	if (new_state == NULL) {
		LOGE(SMF_TAG,"new_state 不能为 NULL");
		INVOKE_ERROR_HOOK(ctx, SMF_ERR_NULL_TRANSITION);
		return;
	}

	/*
	 * 在状态的退出阶段调用 smf_set_state 是没有意义的，
	 * 因为我们已经处于转换过程中；我们将始终忽略 intended 要转换到的状态。
	 */
	if (internal->is_exit) {
		LOGE(SMF_TAG,"在退出动作中调用 %s", __func__);
		INVOKE_ERROR_HOOK(ctx, SMF_ERR_TRANSITION_IN_EXIT);
		return;
	}

#ifdef CONFIG_SMF_ANCESTOR_SUPPORT
	const struct smf_state *topmost;

	if (ctx->executing != new_state && ctx->executing->parent == new_state->parent) {
		/* 优化兄弟状态转换（同一父状态下的不同状态） */
		topmost = ctx->executing->parent;
	} else if (is_descendant_of(ctx->executing, new_state)) {
		/* 新状态是我们当前所在状态的父状态 */
		topmost = new_state;
	} else if (is_descendant_of(new_state, ctx->executing)) {
		/* 我们是新状态的父状态 */
		topmost = ctx->executing;
	} else {
		/* 没有直接关系，查找 LCA */
		topmost = get_lca_of(ctx->executing, new_state);
	}

	internal->is_exit = true;
	internal->new_state = true;

	/* 调用直到（但不包括）topmost 的所有退出动作 */
	if (smf_execute_all_exit_actions(ctx, topmost)) {
		/* 如果在退出动作中设置了终止标志，无需继续 */
		return;
	}

	/* 如果是自转换，调用退出动作 */
	if ((ctx->executing == new_state) && (new_state->exit)) {
		INVOKE_ACTION_HOOK(ctx, new_state, SMF_ACTION_EXIT);
		new_state->exit(ctx);

		/* 如果在退出动作中设置了终止标志，无需继续 */
		if (internal->terminate) {
			return;
		}
	}

	internal->is_exit = false;

	/* 如果是自转换，调用入口动作 */
	if ((ctx->executing == new_state) && (new_state->entry)) {
		INVOKE_ACTION_HOOK(ctx, new_state, SMF_ACTION_ENTRY);
		new_state->entry(ctx);

		/* 如果在入口动作中设置了终止标志，无需继续 */
		if (internal->terminate) {
			return;
		}
	}
#ifdef CONFIG_SMF_INITIAL_TRANSITION
	/*
	 * 最终目标将是目标状态包含的最深层叶状态。
	 * 将其设置为真实目标。
	 */
	while (new_state->initial != NULL) {
		new_state = new_state->initial;
	}
#endif

	/* 更新状态变量 */
	ctx->previous = ctx->current;
	ctx->current = new_state;
	ctx->executing = new_state;

	INVOKE_TRANSITION_HOOK(ctx, ctx->previous, ctx->current);

	/* 调用所有入口动作（除了 topmost 的动作） */
	if (smf_execute_all_entry_actions(ctx, new_state, topmost)) {
		/* 如果在入口动作中设置了终止标志，无需继续 */
		return;
	}
#else
	/* 扁平状态机具有非常简单的转换： */
	if (ctx->current->exit) {
		internal->is_exit = true;
		INVOKE_ACTION_HOOK(ctx, ctx->current, SMF_ACTION_EXIT);
		ctx->current->exit(ctx);
		/* 如果在退出动作中设置了终止标志，无需继续 */
		if (internal->terminate) {
			return;
		}
		internal->is_exit = false;
	}
	/* 更新状态变量 */
	ctx->previous = ctx->current;
	ctx->current = new_state;

	INVOKE_TRANSITION_HOOK(ctx, ctx->previous, ctx->current);

	if (ctx->current->entry) {
		INVOKE_ACTION_HOOK(ctx, ctx->current, SMF_ACTION_ENTRY);
		ctx->current->entry(ctx);
		/* 如果在入口动作中设置了终止标志，无需继续 */
		if (internal->terminate) {
			return;
		}
	}
#endif
}

void smf_set_terminate(struct smf_ctx *const ctx, int32_t val)
{
	struct internal_ctx *const internal = (void *)&ctx->internal;

	internal->terminate = true;
	ctx->terminate_val = val;
}

int32_t smf_run_state(struct smf_ctx *const ctx)
{
	struct internal_ctx *const internal = (void *)&ctx->internal;

	/* 如果设置了终止标志，无需继续 */
	if (internal->terminate) {
		return ctx->terminate_val;
	}

	/* 执行状态的 run 函数可能会导致转换，因此清除
	 * 内部状态以确保正确处理转换。
	 */
	smf_clear_internal_state(ctx);

#ifdef CONFIG_SMF_ANCESTOR_SUPPORT
	ctx->executing = ctx->current;
	if (ctx->current->run) {
		INVOKE_ACTION_HOOK(ctx, ctx->current, SMF_ACTION_RUN);
		enum smf_state_result rc = ctx->current->run(ctx);

		if (rc == SMF_EVENT_HANDLED) {
			internal->handled = true;
		}
	}

	if (smf_execute_ancestor_run_actions(ctx)) {
		return ctx->terminate_val;
	}
#else
	if (ctx->current->run) {
		INVOKE_ACTION_HOOK(ctx, ctx->current, SMF_ACTION_RUN);
		ctx->current->run(ctx);
	}
#endif
	return 0;
}

#ifdef CONFIG_SMF_INSTRUMENTATION
void smf_set_hooks(struct smf_ctx *const ctx, const struct smf_hooks *hooks)
{
	ctx->hooks = hooks;
}
#endif /* CONFIG_SMF_INSTRUMENTATION */