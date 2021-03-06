//===--- DefaultArgumentKind.h - Default Argument Kind Enum -----*- C++ -*-===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2017 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//
// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2019 polarphp software foundation
// Copyright (c) 2017 - 2019 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://polarphp.org/LICENSE.txt for license information
// See https://polarphp.org/CONTRIBUTORS.txt for the list of polarphp project authors
//
// Created by polarboy on 2019/04/25.
//===----------------------------------------------------------------------===//
// This file defines the DefaultArgumentKind enumeration.
//
//===----------------------------------------------------------------------===//

#ifndef POLARPHP_AST_DEFAULT_ARGUMENT_KIND_H
#define POLARPHP_AST_DEFAULT_ARGUMENT_KIND_H

#include <cstdint>

namespace polar::ast {

class Expr;

/// Describes the kind of default argument a tuple pattern element has.
enum class DefaultArgumentKind : uint8_t
{
   /// No default argument.
   None,
   /// A normal default argument.
   Normal,
   /// The default argument is inherited from the corresponding argument of the
   /// overridden declaration.
   Inherited,
   /// The #file default argument, which is expanded at the call site.
   File,
   /// The #line default argument, which is expanded at the call site.
   Line,
   /// The #column default argument, which is expanded at the call site.
   Column,
   /// The #function default argument, which is expanded at the call site.
   Function,
   /// The #dsohandle default argument, which is expanded at the call site.
   DSOHandle,
   /// The "nil" literal.
   NilLiteral,
   /// An empty array literal.
   EmptyArray,
   /// An empty dictionary literal.
   EmptyDictionary,
};
enum { NumDefaultArgumentKindBits = 4 };

} // polar::ast

#endif // POLARPHP_AST_DEFAULT_ARGUMENT_KIND_H
