//===--- trivia.h - Swift Syntax trivia Interface ---------------*- C++ -*-===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2018 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//
//
// This file defines a data structure representing "trivia" in the Swift
// language, such as formatting text like whitespace, or other pieces of
// syntax that don't affect program behavior, like comments.
//
// All source trivia except for comments are kind of "run-length encoded".
// For example, a token might follow 2 newlines and 2 spaces, like so:
//
// func foo() {
//   var x = 2
// }
//
// Here, the 'var' keyword would have the following "leading" trivia:
// [ Newlines(2), Spaces(2) ]
//
// and the following "trailing" trivia:
// [ Spaces(1) ]
//
// Every terminal token in the tree has "leading" and "trailing" trivia.
//
// There is one basic rule to follow when attaching trivia:
//
// 1. A token owns all of its trailing trivia up to, but not including,
//    the next newline character.
//
// 2. Looking backward in the text, a token owns all of the leading trivia
//    up to and including the first contiguous sequence of newlines characters.
//
// For this example again:
//
// func foo() {
//   var x = 2
// }
//
// 'func'
// - Has no leading trivia.
// - Takes up the space after because of rule 1.
// - Leading: [] Trailing: [ Space(1) ]
//
// 'foo'
// - Has no leading trivia. 'func' ate it as its trailing trivia.
// - Has no trailing trivia, because it is butted up against the next '('.
// - Leading: [] Trailing: []
//
// '('
// - Has no leading or trailing trivia.
// - Leading: [] Trailing: []
//
// ')'
// - Has no leading trivia.
// - Takes up the space after because of rule 1.
// - Leading: [] Trailing: [ Space(1) ]
//
// '{'
// - Has no leading trivia. ')' ate it as its trailing trivia.
// - Has no trailing trivia. Because of Rule 1, it doesn't take the newline.
// - Leading: [] Trailing: []
//
// 'var'
// - Takes the newline and preceding two spaces because of Rule 2.
// - Takes the single space that follows because of Rule 1.
// - Leading: [ Newline(1), Space(2) ] Trailing: [ Space(1) ]
//
// ... and so on.
//
//===----------------------------------------------------------------------===//
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
// Created by polarboy on 2019/05/08.

#ifndef POLARPHP_SYNTAX_TRIVIA_H
#define POLARPHP_SYNTAX_TRIVIA_H

#include "polarphp/basic/adt/OwnedString.h"
#include "polarphp/basic/ByteTreeSerialization.h"
#include "polarphp/basic/adt/FoldingSet.h"
#include "polarphp/utils/RawOutStream.h"
#include "polarphp/utils/yaml/YamlTraits.h"
#include "polarphp/global/Global.h"

#include <vector>

namespace polar::syntax {

class AbsolutePosition;

using polar::basic::StringRef;
using polar::basic::FoldingSetNodeId;
using polar::basic::OwnedString;
using polar::utils::RawOutStream;

/// The kind of source trivia, such as spaces, newlines, or comments.
enum class TriviaKind : uint8_t
{
   // A space ' ' character.
   Space,
   // A tab '\t' character.
   Tab,
   // A vertical tab '\v' character.
   VerticalTab,
   // A form-feed '\f' character.
   Formfeed,
   // A newline '\n' character.
   Newline,
   // A newline '\r' character.
   CarriageReturn,
   // A newline consists of contiguous '\r' and '\n' characters.
   CarriageReturnLineFeed,
   // A backtick '`' character, used to escape identifiers.
   Backtick,
   // A developer line comment, starting with "//"
   LineComment,
   // A developer block comment, starting with "/*" and ending with "*/".
   BlockComment,
   // A documentation line comment, starting with "///".
   DocLineComment,
   // A documentation block comment, starting with "/**" and ending with "*/".
   DocBlockComment,
   // Any skipped garbage text.
   GarbageText
};

bool is_comment_trivia_kind(TriviaKind kind);

/// A contiguous stretch of a single kind of trivia. The constituent part of
/// a `trivia` collection.
///
/// For example, four spaces would be represented by
/// { TriviaKind::Space, 4, "" }.
///
/// All trivia except for comments don't need to store text, since they can be
/// reconstituted using their m_kind and m_count.
///
/// In general, you should deal with the actual trivia collection instead
/// of individual pieces whenever possible.
class TriviaPiece
{
public:

   /// single char trivia
   static TriviaPiece getSpace(const OwnedString text)
   {
      return {TriviaKind::Space, text};
   }

   static TriviaPiece getTab(const OwnedString text)
   {
      return {TriviaKind::Tab, text};
   }

   static TriviaPiece getVerticalTab(const OwnedString text)
   {
      return {TriviaKind::VerticalTab, text};
   }

   static TriviaPiece getFormfeed(const OwnedString text)
   {
      return {TriviaKind::Formfeed, text};
   }

   static TriviaPiece getNewline(const OwnedString text)
   {
      return {TriviaKind::Newline, text};
   }

   static TriviaPiece getCarriageReturn(const OwnedString text)
   {
      return {TriviaKind::CarriageReturn, text};
   }

   static TriviaPiece getBacktick(const OwnedString text)
   {
      return {TriviaKind::Backtick, text};
   }

   static TriviaPiece getLineComment(const OwnedString text)
   {
      return {TriviaKind::LineComment, text};
   }

   static TriviaPiece getBlockComment(const OwnedString text)
   {
      return {TriviaKind::BlockComment, text};
   }

   static TriviaPiece getDocLineComment(const OwnedString text)
   {
      return {TriviaKind::DocLineComment, text};
   }

   static TriviaPiece getDocBlockComment(const OwnedString text)
   {
      return {TriviaKind::DocBlockComment, text};
   }

   static TriviaPiece getGarbageText(const OwnedString text)
   {
      return {TriviaKind::GarbageText, text};
   }

   /// multi char trivia
   static TriviaPiece getCarriageReturnLineFeeds(unsigned count)
   {
      return {TriviaKind::CarriageReturnLineFeed, count};
   }

   static TriviaPiece getCarriageReturnLineFeed()
   {
      return getCarriageReturnLineFeeds(1);
   }

   static TriviaPiece fromText(TriviaKind kind, StringRef text);

   /// Return kind of the trivia.
   TriviaKind getKind() const
   {
      return m_kind;
   }

   /// Return the text of the trivia.
   StringRef getText() const
   {
      return m_text.str();
   }

   /// Return the text of the trivia.
   unsigned getCount() const
   {
      return m_count;
   }

   /// Return textual length of the trivia.
   size_t getTextLength() const
   {
      switch (m_kind) {
      case TriviaKind::Space:
      case TriviaKind::Tab:
      case TriviaKind::VerticalTab:
      case TriviaKind::Formfeed:
      case TriviaKind::Newline:
      case TriviaKind::CarriageReturn:
      case TriviaKind::Backtick:
      case TriviaKind::LineComment:
      case TriviaKind::BlockComment:
      case TriviaKind::DocLineComment:
      case TriviaKind::DocBlockComment:
      case TriviaKind::GarbageText:
         return m_text.getSize();
      case TriviaKind::CarriageReturnLineFeed:
         return m_count * 2;
      }
      polar_unreachable("unhandled kind");
   }

   bool isComment() const
   {
      return is_comment_trivia_kind(getKind());
   }

   void accumulateAbsolutePosition(AbsolutePosition &pos) const;

   /// Try to compose this and next to one TriviaPiece.
   /// It returns true if it is succeeded.
   bool trySquash(const TriviaPiece &next);

   /// Print a debug representation of this trivia piece to the provided output
   /// stream and indentation level.
   void dump(RawOutStream &outStream, unsigned indent = 0) const;

   /// Print this piece of trivia to the provided output stream.
   void print(RawOutStream &outStream) const;

   bool operator==(const TriviaPiece &other) const
   {
      return m_kind == other.m_kind &&
            m_count == other.m_count &&
            m_text.str().compare(other.m_text.str()) == 0;
   }

   bool operator!=(const TriviaPiece &other) const
   {
      return !(*this == other);
   }

   void getProfile(FoldingSetNodeId &id) const
   {
      id.addInteger(unsigned(m_kind));
      switch (m_kind) {
      case TriviaKind::Space:
      case TriviaKind::Tab:
      case TriviaKind::VerticalTab:
      case TriviaKind::Formfeed:
      case TriviaKind::Newline:
      case TriviaKind::CarriageReturn:
      case TriviaKind::Backtick:
      case TriviaKind::LineComment:
      case TriviaKind::BlockComment:
      case TriviaKind::DocLineComment:
      case TriviaKind::DocBlockComment:
      case TriviaKind::GarbageText:
         id.addString(m_text.getStr());
         break;
      case TriviaKind::CarriageReturnLineFeed:
         id.addInteger(m_count);
         break;
      }
   }

private:
   TriviaPiece(const TriviaKind kind, const OwnedString text)
      : m_kind(kind),
        m_count(1),
        m_text(text)
   {}

   TriviaPiece(const TriviaKind kind, const unsigned count)
      : m_kind(kind),
        m_count(count),
        m_text()
   {}

   friend struct polar::yaml::MappingTraits<TriviaPiece>;

   TriviaKind m_kind;
   unsigned m_count;
   OwnedString m_text;
};

using TriviaList = std::vector<TriviaPiece>;

/// A collection of leading or trailing Trivia. This is the main data structure
/// for thinking about Trivia.
struct Trivia
{
   TriviaList pieces;

   /// Get the begin iterator of the pieces.
   TriviaList::const_iterator begin() const
   {
      return pieces.begin();
   }

   /// Get the end iterator of the pieces.
   TriviaList::const_iterator end() const
   {
      return pieces.end();
   }

   /// Add a piece to the end of the collection.
   void push_back(const TriviaPiece &piece)
   {
      pieces.push_back(piece);
   }

   /// Add a piece to the beginning of the collection.
   void push_front(const TriviaPiece &piece)
   {
      pieces.insert(pieces.begin(), piece);
   }

   /// Add a piece to the end of the collection.
   void pushBack(const TriviaPiece &piece)
   {
      pieces.push_back(piece);
   }

   /// Add a piece to the beginning of the collection.
   void pushFront(const TriviaPiece &piece)
   {
      pieces.insert(pieces.begin(), piece);
   }

   /// Clear pieces.
   void clear()
   {
      pieces.clear();
   }

   /// Return a reference to the first piece.
   ///
   /// Precondition: !empty()
   const TriviaPiece &front() const
   {
      assert(!empty());
      return pieces.front();
   }

   /// Return a reference to the last piece.
   ///
   /// Precondition: !empty()
   const TriviaPiece &back() const
   {
      assert(!empty());
      return pieces.back();
   }

   /// Remove the last piece from the trivia collection.
   ///
   /// Precondition: !empty()
   void pop_back()
   {
      assert(!empty());
      pieces.pop_back();
   }

   /// Returns true if there are no pieces in this trivia collection.
   bool empty() const
   {
      return pieces.empty();
   }

   /// Return the number of pieces in this trivia collection.
   size_t size() const
   {
      return pieces.size();
   }

   size_t getTextLength() const
   {
      size_t len = 0;
      for (auto &piece : pieces) {
         len += piece.getTextLength();
      }
      return len;
   }

   /// Append next TriviaPiece or compose last TriviaPiece and
   /// next TriviaPiece to one last TriviaPiece if it can.
   void appendOrSquash(const TriviaPiece &next);

   /// Dump a debug representation of this trivia collection to standard error.
   void dump() const;

   /// Dump a debug representation of this trivia collection to the provided
   /// stream and indentation level.
   void dump(RawOutStream &outStream, unsigned indent = 0) const;

   /// Print all of the pieces to the provided output stream in source order.
   void print(RawOutStream &outStream) const;

   /// Return a new trivia collection by appending pieces from `other`.
   Trivia appending(const Trivia &other) const;
   Trivia operator+(const Trivia &other) const;

   /// Look for the first TriviaPiece with the desiredKind. If not found,
   /// returns the end iterator.
   TriviaList::const_iterator find(const TriviaKind desiredKind) const;

   /// Returns true if the trivia collection contains a piece of the given kind.
   bool contains(const TriviaKind kind) const
   {
      return find(kind) != end();
   }

   bool operator==(const Trivia &other) const
   {
      if (pieces.size() != other.size()) {
         return false;
      }

      for (size_t i = 0; i < pieces.size(); ++i) {
         if (pieces[i] != other.pieces[i]) {
            return false;
         }
      }
      return true;
   }

   bool operator!=(const Trivia &other) const
   {
      return !(*this == other);
   }

   /// single char trivia
   static Trivia getSpace(const OwnedString text)
   {
      return {{TriviaPiece::getSpace(text)}};
   }

   static Trivia getTab(const OwnedString text)
   {
      return {{TriviaPiece::getTab(text)}};
   }

   static Trivia getVerticalTab(const OwnedString text)
   {
      return {{TriviaPiece::getVerticalTab(text)}};
   }

   static Trivia getFormfeed(const OwnedString text)
   {
      return {{TriviaPiece::getFormfeed(text)}};
   }

   static Trivia getNewline(const OwnedString text)
   {
      return {{TriviaPiece::getNewline(text)}};
   }

   static Trivia getCarriageReturn(const OwnedString text)
   {
      return {{TriviaPiece::getCarriageReturn(text)}};
   }

   static Trivia getBacktick(const OwnedString text)
   {
      return {{TriviaPiece::getBacktick(text)}};
   }

   static Trivia getLineComment(const OwnedString text)
   {
      return {{TriviaPiece::getLineComment(text)}};
   }

   static Trivia getBlockComment(const OwnedString text)
   {
      return {{TriviaPiece::getBlockComment(text)}};
   }

   static Trivia getDocLineComment(const OwnedString text)
   {
      return {{TriviaPiece::getDocLineComment(text)}};
   }

   static Trivia getDocBlockComment(const OwnedString text)
   {
      return {{TriviaPiece::getDocBlockComment(text)}};
   }

   static Trivia getGarbageText(const OwnedString text)
   {
      return {{TriviaPiece::getGarbageText(text)}};
   }

   /// multi char trivia
   static Trivia getCarriageReturnLineFeeds(unsigned count)
   {
      return {{TriviaPiece::getCarriageReturnLineFeeds(count)}};
   }

   static Trivia getCarriageReturnLineFeed()
   {
      return {{TriviaPiece::getCarriageReturnLineFeeds(1)}};
   }

private:
   static bool checkTriviaText(StringRef text, TriviaKind kind)
   {
      switch(kind) {
      case TriviaKind::LineComment:
         return text.startsWith("//");
      case TriviaKind::BlockComment:
         return text.startsWith("/*") && text.endsWith("*/");
      case TriviaKind::DocLineComment:
         return text.startsWith("///");
      case TriviaKind::DocBlockComment:
         return text.startsWith("/**") && text.endsWith("*/");
      case TriviaKind::GarbageText:
         return !text.empty();
      case TriviaKind::CarriageReturnLineFeed:
         return true;
      default:
         polar_unreachable("unexcepted kind");
      }
   }
};
} // polar::syntax

namespace polar::basic::bytetree {

using polar::syntax::TriviaKind;

template <>
struct WrapperTypeTraits<syntax::TriviaKind>
{
   static uint8_t numericValue(const syntax::TriviaKind &kind)
   {
      return polar::as_integer<TriviaKind>(kind);
   }

   static void write(ByteTreeWriter &writer, const syntax::TriviaKind &kind,
                     unsigned index)
   {
      writer.write(numericValue(kind), index);
   }
};

template <>
struct ObjectTraits<syntax::TriviaPiece>
{
   static unsigned numFields(const syntax::TriviaPiece &trivia,
                             UserInfoMap &userInfo)
   {
      return 2;
   }

   static void write(ByteTreeWriter &writer, const syntax::TriviaPiece &trivia,
                     UserInfoMap &userInfo)
   {
      using polar::syntax::TriviaKind;
      writer.write(trivia.getKind(), /*index=*/0);
      // Write the trivia's text or count depending on its kind
      switch (trivia.getKind()) {
      case TriviaKind::Space:
      case TriviaKind::Tab:
      case TriviaKind::VerticalTab:
      case TriviaKind::Formfeed:
      case TriviaKind::Newline:
      case TriviaKind::CarriageReturn:
      case TriviaKind::Backtick:
      case TriviaKind::LineComment:
      case TriviaKind::BlockComment:
      case TriviaKind::DocLineComment:
      case TriviaKind::DocBlockComment:
      case TriviaKind::GarbageText:
         writer.write(trivia.getText(), /*Index=*/ 1);
         break;
      case TriviaKind::CarriageReturnLineFeed:
         writer.write(static_cast<uint32_t>(trivia.getCount()), /*Index=*/1);
         break;
      }
   }
};

} // polar::basic::bytetree

namespace polar::yaml {

/// Deserialization traits for TriviaPiece.
/// - All trivia pieces will have a "kind" key that contains the serialized
///   name of the trivia kind.
/// - Comment trivia will have the associated text of the comment under the
///   "value" key.
/// - All other trivia will have the associated integer count of their
///   occurrences under the "value" key.
template<>
struct MappingTraits<polar::syntax::TriviaPiece>
{
   static polar::syntax::TriviaPiece mapping(IO &in)
   {
      using polar::syntax::TriviaKind;
      TriviaKind kind;
      in.mapRequired("kind", kind);
      switch (kind) {
      case TriviaKind::Space:
      case TriviaKind::Tab:
      case TriviaKind::VerticalTab:
      case TriviaKind::Formfeed:
      case TriviaKind::Newline:
      case TriviaKind::CarriageReturn:
      case TriviaKind::Backtick:
      case TriviaKind::LineComment:
      case TriviaKind::BlockComment:
      case TriviaKind::DocLineComment:
      case TriviaKind::DocBlockComment:
      case TriviaKind::GarbageText: {
         StringRef text;
         in.mapRequired("value", text);
         return polar::syntax::TriviaPiece(
                  kind, polar::basic::OwnedString::makeRefCounted(text));
      }
      case TriviaKind::CarriageReturnLineFeed:
      {
         /// FIXME: This is a workaround for existing bug from llvm yaml parser
         /// which would raise error when deserializing number with trailing character
         /// like "1\n". See https://bugs.llvm.org/show_bug.cgi?id=15505
         StringRef str;
         in.mapRequired("value", str);
         unsigned count = std::atoi(str.data());
         return polar::syntax::TriviaPiece(kind, count);
      }
      }
   }
};

/// Deserialization traits for TriviaKind.
template <>
struct ScalarEnumerationTraits<polar::syntax::TriviaKind>
{
   static void enumeration(IO &in, polar::syntax::TriviaKind &value)
   {
      in.enumCase(value, "Space", polar::syntax::TriviaKind::Space);
      in.enumCase(value, "Tab", polar::syntax::TriviaKind::Tab);
      in.enumCase(value, "VerticalTab", polar::syntax::TriviaKind::VerticalTab);
      in.enumCase(value, "Formfeed", polar::syntax::TriviaKind::Formfeed);
      in.enumCase(value, "Newline", polar::syntax::TriviaKind::Newline);
      in.enumCase(value, "CarriageReturn", polar::syntax::TriviaKind::CarriageReturn);
      in.enumCase(value, "Backtick", polar::syntax::TriviaKind::Backtick);
      in.enumCase(value, "LineComment", polar::syntax::TriviaKind::LineComment);
      in.enumCase(value, "BlockComment", polar::syntax::TriviaKind::BlockComment);
      in.enumCase(value, "DocLineComment", polar::syntax::TriviaKind::DocLineComment);
      in.enumCase(value, "DocBlockComment", polar::syntax::TriviaKind::DocBlockComment);
      in.enumCase(value, "GarbageText", polar::syntax::TriviaKind::GarbageText);
      in.enumCase(value, "CarriageReturnLineFeed", polar::syntax::TriviaKind::CarriageReturnLineFeed);
   }
};

} // polar::yaml

#endif // POLARPHP_SYNTAX_TRIVIA_H
