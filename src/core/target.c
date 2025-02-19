/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2022 Meta Platforms, Inc. and affiliates.
 */

#include "target.h"

#include <linux/bpf.h>

#include <errno.h>
#include <stdlib.h>

#include "core/flavor.h"
#include "core/hook.h"
#include "generator/codegen.h"
#include "generator/program.h"
#include "generator/reg.h"
#include "shared/helper.h"

#include "external/filter.h"

const char *bf_target_type_to_str(enum bf_target_type type)
{
    static const char *str[] = {
        [BF_TARGET_TYPE_STANDARD] = "STANDARD",
        [BF_TARGET_TYPE_ERROR] = "ERROR",
    };

    bf_assert(0 <= type && type < _BF_TARGET_TYPE_MAX);
    static_assert(ARRAY_SIZE(str) == _BF_TARGET_TYPE_MAX);

    return str[type];
}

const char *
bf_target_standard_verdict_to_str(enum bf_target_standard_verdict verdict)
{
    static const char *str[] = {
        [BF_TARGET_STANDARD_ACCEPT] = "ACCEPT",
        [BF_TARGET_STANDARD_DROP] = "DROP",
    };

    bf_assert(0 <= verdict && verdict < _BF_TARGET_STANDARD_MAX);
    static_assert(ARRAY_SIZE(str) == _BF_TARGET_STANDARD_MAX);

    return str[verdict];
}

int bf_target_new(struct bf_target **target)
{
    _cleanup_bf_target_ struct bf_target *_target = NULL;

    bf_assert(target);

    _target = calloc(1, sizeof(struct bf_target));
    if (!_target)
        return -ENOMEM;

    *target = TAKE_PTR(_target);

    return 0;
}

void bf_target_free(struct bf_target **target)
{
    if (!*target)
        return;

    free(*target);
    *target = NULL;
}

int bf_target_generate_standard(struct bf_program *program,
                                const struct bf_target *target)
{
    /// @todo Support target jumping to user-defined chains.

    const struct bf_flavor_ops *ops =
        bf_flavor_ops_get(bf_hook_to_flavor(program->hook));

    EMIT(program,
         BPF_MOV32_IMM(BF_REG_RET, ops->convert_return_code(target->verdict)));

    EMIT(program, BPF_EXIT_INSN());

    return 0;
}

int bf_target_generate_error(struct bf_program *program,
                             const struct bf_target *target)
{
    UNUSED(program);
    UNUSED(target);

    return -ENOTSUP;
}

const struct bf_target_ops *bf_target_ops_get(enum bf_target_type type)
{
    static const struct bf_target_ops target_ops[] = {
        [BF_TARGET_TYPE_STANDARD] =
            {
                .generate = bf_target_generate_standard,
            },
        [BF_TARGET_TYPE_ERROR] =
            {
                .generate = bf_target_generate_error,
            },
    };

    bf_assert(0 <= type && type < _BF_TARGET_TYPE_MAX);
    static_assert(ARRAY_SIZE(target_ops) == _BF_TARGET_TYPE_MAX);

    return &target_ops[type];
}
