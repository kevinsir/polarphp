//===--- AccessorKinds.def - Swift accessor metaprogramming -----*- C++ -*-===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2017 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2019 polarphp software foundation
// Copyright (c) 2017 - 2019 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://polarphp.org/LICENSE.txt for license information
// See https://polarphp.org/CONTRIBUTORS.txt for the list of polarphp project authors
//
// Created by polarboy on 2019/04/30.
//===----------------------------------------------------------------------===//
//
// This file defines macros used for macro-metaprogramming with accessor kinds.
//
//===----------------------------------------------------------------------===//

#if defined(ACCESSOR) && defined(ACCESSOR_KEYWORD)
#error do not define both ACCESSOR and ACCESSOR_KEYWORD
#elif !defined(ACCESSOR) && !defined(ACCESSOR_KEYWORD)
#error must define either ACCESSOR or ACCESSOR_KEYWORD
#endif

/// ACCESSOR(ID)
///   There is an accessor with the enumerator value AccessorKind::ID.
#if !defined(ACCESSOR) && defined(ACCESSOR_KEYWORD)
#define ACCESSOR(ID)
#endif

/// SINGLETON_ACCESSOR(ID, KEYWORD)
///   The given accessor has only one matching accessor keyword.
///
/// Defaults to ACCESSOR(ID) or ACCESSOR_KEYWORD(KEYWORD), depending on which
/// is defined.
#ifndef SINGLETON_ACCESSOR
#if defined(ACCESSOR_KEYWORD)
#define SINGLETON_ACCESSOR(ID, KEYWORD) ACCESSOR_KEYWORD(KEYWORD)
#else
#define SINGLETON_ACCESSOR(ID, KEYWORD) ACCESSOR(ID)
#endif
#endif

/// OBSERVING_ACCESSOR(ID, KEYWORD)
///   The given accessor is an observing accessor.
///
///   Defaults to SINGLETON_ACCESSOR(ID, KEYWORD).
#ifndef OBSERVING_ACCESSOR
#define OBSERVING_ACCESSOR(ID, KEYWORD) SINGLETON_ACCESSOR(ID, KEYWORD)
#endif

/// OPAQUE_ACCESSOR(ID, KEYWORD)
///   The given accessor is used in the opaque access pattern and is created
///   for (mutable) storage whenever its implementation is unknown, such as
///   when it is resilient, overridable, or accessed through a protocol.
///
///   Defaults to SINGLETON_ACCESSOR(ID, KEYWORD).
#ifndef OPAQUE_ACCESSOR
#define OPAQUE_ACCESSOR(ID, KEYWORD) SINGLETON_ACCESSOR(ID, KEYWORD)
#endif

/// COROUTINE_ACCESSOR(ID, KEYWORD)
///   The given accessor is a coroutine accessor, i.e. a reader or modifier.
///
///   Defaults to OPAQUE_ACCESSOR(ID, KEYWORD).
#ifndef COROUTINE_ACCESSOR
#define COROUTINE_ACCESSOR(ID, KEYWORD) OPAQUE_ACCESSOR(ID, KEYWORD)
#endif

/// ANY_ADDRESSOR(ID, KEYWORD)
///   The given keyword corresponds to an addressor of the given kind.
///
///   Defaults to SINGLETON_ACCESSOR(ID, KEYWORD).
#ifndef ANY_ADDRESSOR
#define ANY_ADDRESSOR(ID, KEYWORD) \
  SINGLETON_ACCESSOR(ID, KEYWORD)
#endif

/// IMMUTABLE_ADDRESSOR(ID, KEYWORD)
///   The given keyword corresponds to an immutable addressor of the given kind.
///
///   DEfaults to ANY_ADDRESSOR(ID, KEYWORD).
#ifndef IMMUTABLE_ADDRESSOR
#define IMMUTABLE_ADDRESSOR(ID, KEYWORD) \
  ANY_ADDRESSOR(ID, KEYWORD)
#endif

/// MUTABLE_ADDRESSOR(ID, KEYWORD)
///   The given keyword corresponds to a mutable addressor of the given kind.
///
///   DEfaults to ANY_ADDRESSOR(ID, KEYWORD).
#ifndef MUTABLE_ADDRESSOR
#define MUTABLE_ADDRESSOR(ID, KEYWORD) \
  ANY_ADDRESSOR(ID, KEYWORD)
#endif

// Suppress entries for accessors which can't be written in source code.
#ifndef SUPPRESS_ARTIFICIAL_ACCESSORS
#define SUPPRESS_ARTIFICIAL_ACCESSORS 0
#endif

/// This is a read accessor: a yield-once coroutine which is called when a
/// value is loaded from the storage, like a getter, but which works
/// by yielding a borrowed value of the storage type.
///
/// If the storage is not implemented with a read accessor then
/// one can always be synthesized (even if the storage type is move-only).
COROUTINE_ACCESSOR(Read, _read);

/// This is a modify accessor: a yield-once coroutine which is called when a
/// the storage is modified which works by yielding an inout value
/// of the storage type.
///
/// If the storage is not implemented with a modify accessor then
/// one can be synthesized if the storage is mutable at all.
COROUTINE_ACCESSOR(Modify, _modify);

/// This is a willSet observer: a function which "decorates" an
/// underlying assignment operation by being called prior to the
/// operation when a value is assigned to the storage.
///
/// willSet is essentially sugar for implementing a certain common
/// setter idiom.
OBSERVING_ACCESSOR(WillSet, willSet);

/// This is a didSet observer: a function which "decorates" an
/// underlying assignment operation by being called after the
/// operation when a value is assigned to the storage.
///
/// didSet is essentially sugar for implementing a certain common
/// setter idiom.
OBSERVING_ACCESSOR(DidSet, didSet);

/// This is an address-family accessor: a function that is called when
/// a value is loaded from the storage, like a getter, but which works
/// by returning a pointer to an immutable value of the storage type.
/// This kind of accessor also has an addressor kind.
///
/// Addressors are a way of proving more efficient access to storage
/// when it is already stored in memory (but not as a stored member
/// of the type).
IMMUTABLE_ADDRESSOR(Address, unsafeAddress);

/// This is a mutableAddress-family accessor: a function that is
/// called when the storage is modified and which works by returning
/// a pointer to a mutable value of the storage type.
/// This kind of accessor also has an addressor kind.
///
/// Addressors are a way of proving more efficient access to storage
/// when it is already stored in memory (but not as a stored member
/// of the type).
MUTABLE_ADDRESSOR(MutableAddress, unsafeMutableAddress);

#ifdef LAST_ACCESSOR
LAST_ACCESSOR(MutableAddress)
#undef LAST_ACCESSOR
#endif

#undef IMMUTABLE_ADDRESSOR
#undef MUTABLE_ADDRESSOR
#undef ANY_ADDRESSOR
#undef OPAQUE_ACCESSOR
#undef COROUTINE_ACCESSOR
#undef OBSERVING_ACCESSOR
#undef SINGLETON_ACCESSOR
#undef ACCESSOR
#undef ACCESSOR_KEYWORD
#undef SUPPRESS_ARTIFICIAL_ACCESSORS
