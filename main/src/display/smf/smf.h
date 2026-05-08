/*
 * Copyright 2021 The Chromium OS Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 *
 * @brief 状态机框架头文件
 */

#ifndef _SMF_H_
#define _SMF_H_

#include <stdint.h>

/**
 * @brief 状态机框架 API
 * @defgroup smf 状态机框架 API
 * @version 0.2.0
 * @ingroup os_services
 * @{
 */

/**
 * @brief 用于创建具有初始转换的层次化状态的宏。
 *
 * @param _entry   状态入口函数或 NULL
 * @param _run     状态运行函数或 NULL
 * @param _exit    状态退出函数或 NULL
 * @param _parent  状态父对象或 NULL
 * @param _initial 状态初始转换对象或 NULL
 */
#define CONFIG_SMF_ANCESTOR_SUPPORT
#define CONFIG_SMF_INITIAL_TRANSITION
#define SMF_CREATE_STATE(_entry, _run, _exit, _parent, _initial)           \
{                                                                          \
    .entry   = _entry,                                                 \
    .run     = _run,                                                   \
    .exit    = _exit,                                                  \
    .parent  = _parent,      /* 强制启用 parent */ \
    .initial = _initial,    /* 强制启用 initial */ \
}
/* clang-format on */

/**
 * @brief 将用户定义的对象强制转换为状态机上下文的宏。
 *
 * @param o 指向用户定义对象的指针
 */
#define SMF_CTX(o) ((struct smf_ctx *)o)

#ifdef __cplusplus
extern "C" {
#endif

// #include <zephyr/kernel.h>

/**
 * @brief 状态执行函数的返回值枚举
 */
enum smf_state_result {
	SMF_EVENT_HANDLED,
	SMF_EVENT_PROPAGATE,
};

/**
 * @brief 标识正在执行的动作类型的枚举。
 * @note 这用于 instrumentation（仪器化/监控）目的。
 */
enum smf_action_type {
	SMF_ACTION_ENTRY, /**< 入口动作 */
	SMF_ACTION_RUN,   /**< 运行动作 */
	SMF_ACTION_EXIT,  /**< 退出动作 */
};

/** 通过 instrumentation 错误钩子报告的错误代码 */
#define SMF_ERR_NULL_TRANSITION    1 /**< smf_set_state 中 new_state 为 NULL */
#define SMF_ERR_TRANSITION_IN_EXIT 2 /**< 在退出动作中调用了 smf_set_state */

#if defined(CONFIG_SMF_INSTRUMENTATION) || defined(__DOXYGEN__)
/* 钩子 typedef 的前向声明 */
struct smf_ctx;
struct smf_state;

/**
 * @brief 在当前状态指针更新后、新状态的入口动作执行前调用。
 *
 * @param ctx    状态机上下文
 * @param source 之前的状态（转换前）
 * @param dest   新的当前状态（转换后）
 */
typedef void (*smf_transition_hook)(struct smf_ctx *ctx, const struct smf_state *source,
				    const struct smf_state *dest);

/**
 * @brief 在状态动作（入口/运行/退出）被调用之前调用。
 *
 * @param ctx         状态机上下文
 * @param state       即将执行动作的状态
 * @param action_type 哪个动作（入口、运行或退出）
 */
typedef void (*smf_action_hook)(struct smf_ctx *ctx, const struct smf_state *state,
				enum smf_action_type action_type);

/**
 * @brief 检测到无效操作时调用。
 *
 * @param ctx        状态机上下文
 * @param error_code SMF_ERR_* 定义之一
 */
typedef void (*smf_error_hook)(struct smf_ctx *ctx, int error_code);

/**
 * @brief 可选的 instrumentation 钩子集合。
 *
 * 任何成员都可以为 NULL 以跳过该通知。
 */
struct smf_hooks {
	smf_transition_hook on_transition; /**< 转换时调用的钩子 */
	smf_action_hook on_action;         /**< 入口/运行/退出动作时调用的钩子 */
	smf_error_hook on_error;           /**< 错误时调用的钩子 */
};
#endif /* CONFIG_SMF_INSTRUMENTATION */

/**
 * @brief 实现状态入口和退出动作的函数指针
 *
 * @param obj 指向用户定义对象的指针
 */
typedef void (*state_method)(void *obj);

/**
 * @brief 实现状态运行动作的函数指针
 *
 * @param obj 指向用户定义对象的指针
 * @return 事件是否应传播到父状态
 *         （当未定义 CONFIG_SMF_ANCESTOR_SUPPORT 时忽略）
 */
typedef enum smf_state_result (*state_execution)(void *obj);

/** 可用于多个状态机的通用状态。 */
struct smf_state {
	/** 进入此状态时将运行的可选方法 */
	const state_method entry;

	/**
	 * 在状态机循环期间重复运行的可选方法。
	 */
	const state_execution run;

	/** 退出此状态时将运行的可选方法 */
	const state_method exit;
#ifdef CONFIG_SMF_ANCESTOR_SUPPORT
	/**
	 * 可选的父状态，包含各种子状态之间通用的入口/运行/退出
	 * 实现。
	 * 入口：父函数在子函数之前执行。
	 * 运行：父函数在子函数之后执行。
	 * 退出：父函数在子函数之后执行。
	 *
	 * 注意：在具有共享父状态的两个子状态之间转换时，
	 *       该父状态的退出和入口函数不会执行。
	 */
	const struct smf_state *parent;

#ifdef CONFIG_SMF_INITIAL_TRANSITION
	/**
	 * 可选的初始转换状态。叶状态为 NULL。
	 */
	const struct smf_state *initial;
#endif /* CONFIG_SMF_INITIAL_TRANSITION */
#endif /* CONFIG_SMF_ANCESTOR_SUPPORT */
};

/** 定义状态机的当前上下文。 */
struct smf_ctx {
	/** 状态机正在执行的当前状态。 */
	const struct smf_state *current;
	/** 状态机执行的上一个状态 */
	const struct smf_state *previous;

#ifdef CONFIG_SMF_ANCESTOR_SUPPORT
	/** 当前正在执行的状态（可能是父状态） */
	const struct smf_state *executing;
#endif /* CONFIG_SMF_ANCESTOR_SUPPORT */
	/**
	 * 此值由 set_terminate 函数设置，
	 * 当 run_state 函数返回非零值时，
	 * 应终止状态机。
	 */
	int32_t terminate_val;
	/**
	 * 状态机将其强制转换为 "struct internal_ctx"，
	 * 用于跟踪状态机上下文
	 */
	uint32_t internal;

#ifdef CONFIG_SMF_INSTRUMENTATION
	/** 用于测试和调试的可选 instrumentation 钩子 */
	const struct smf_hooks *hooks;
#endif /* CONFIG_SMF_INSTRUMENTATION */
};

/**
 * @brief 初始化状态机并设置其初始状态。
 *
 * @param ctx        状态机上下文
 * @param init_state 状态机启动时的初始状态。
 */
void smf_set_initial(struct smf_ctx *ctx, const struct smf_state *init_state);

/**
 * @brief 更改状态机的状态。这处理退出上一个状态
 *        并进入目标状态。对于 HSM（层次状态机），
 *        最小公共祖先 (LCA) 的入口和退出动作不会运行。
 *
 * @param ctx       状态机上下文
 * @param new_state 要转换到的状态（NULL 有效，表示退出所有状态）
 */
void smf_set_state(struct smf_ctx *ctx, const struct smf_state *new_state);

/**
 * @brief 终止状态机
 *
 * @param ctx  状态机上下文
 * @param val  非零终止值，由 smf_run_state
 *             函数返回。
 */
void smf_set_terminate(struct smf_ctx *ctx, int32_t val);

#ifdef CONFIG_SMF_INSTRUMENTATION
/**
 * @brief 在状态机上下文上设置 instrumentation 钩子。
 *
 * 必须在 **smf_set_initial() 之后** 调用，因为 smf_set_initial()
 * 会将钩子指针重置为 NULL。在 smf_set_initial() 期间执行的入口动作
 * （初始状态及其祖先）不会被这些钩子捕获。
 *
 * @param ctx   状态机上下文
 * @param hooks 指向钩子结构体的指针，或 NULL 以禁用钩子。
 *              指向的结构体必须比状态机存活更久。
 */
void smf_set_hooks(struct smf_ctx *ctx, const struct smf_hooks *hooks);
#endif /* CONFIG_SMF_INSTRUMENTATION */

/**
 * @brief 运行状态机的一次迭代（包括任何父状态）
 *
 * @param ctx  状态机上下文
 * @return	   非零值应终止状态机。此
 *			   非零值可以表示达到了终止状态
 *			   或检测到应导致状态机
 *			   终止的错误。
 */
int32_t smf_run_state(struct smf_ctx *ctx);

/**
 * @brief 获取当前的叶状态。
 *
 * @note 如果 HSM 格式不正确
 *		 （即初始转换设置不正确），这可能是父状态。
 *
 * @param ctx 状态机上下文
 * @return    当前的叶状态。
 */
static inline const struct smf_state *smf_get_current_leaf_state(const struct smf_ctx *const ctx)
{
	return ctx->current;
}

/**
 * @brief 获取当前正在执行的状态。这可能是父状态。
 *
 * @param ctx 状态机上下文
 * @return    当前正在执行的状态。
 */
static inline const struct smf_state *
smf_get_current_executing_state(const struct smf_ctx *const ctx)
{
#ifdef CONFIG_SMF_ANCESTOR_SUPPORT
	return ctx->executing;
#else
	return ctx->current;
#endif /* CONFIG_SMF_ANCESTOR_SUPPORT */
}

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* ZEPHYR_INCLUDE_SMF_H_ */