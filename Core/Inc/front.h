/*
 * front.h
 *
 *  Created on: Nov 4, 2013
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#ifndef FRONT_H_
#define FRONT_H_

//js does not work with latest GCC
#define DISABLE_CONV_WARNING _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wconversion\"")
#define DISABLE_SIGN_WARNING _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wsign-conversion\"")
#define END_DISABLE_WARNING _Pragma("GCC diagnostic pop")
#define END_DISABLE_CONV_WARNING END_DISABLE_WARNING


#endif /* FRONT_H_ */
