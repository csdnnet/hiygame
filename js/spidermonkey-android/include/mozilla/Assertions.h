/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sw=4 et tw=99 ft=cpp:
 *
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Code.
 *
 * The Initial Developer of the Original Code is
 *   The Mozilla Foundation
 * Portions created by the Initial Developer are Copyright (C) 2011
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Jeff Walden <jwalden+code@mit.edu>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

/* Implementations of runtime and static assertion macros for C and C++. */

#ifndef mozilla_Assertions_h_
#define mozilla_Assertions_h_

#include "mozilla/Attributes.h"
#include "mozilla/Types.h"

/*
 * MOZ_STATIC_ASSERT may be used to assert a condition *at compile time*.  This
 * can be useful when you make certain assumptions about what must hold for
 * optimal, or even correct, behavior.  For example, you might assert that the
 * size of a struct is a multiple of the target architecture's word size:
 *
 *   struct S { ... };
 *   MOZ_STATIC_ASSERT(sizeof(S) % sizeof(size_t) == 0,
 *                     "S should be a multiple of word size for efficiency");
 *
 * This macro can be used in any location where both an extern declaration and a
 * typedef could be used.
 *
 * Be aware of the gcc 4.2 concerns noted further down when writing patches that
 * use this macro, particularly if a patch only bounces on OS X.
 */
#ifdef __cplusplus
#  if defined(__clang__)
#    ifndef __has_extension
#      define __has_extension __has_feature /* compatibility, for older versions of clang */
#    endif
#    if __has_extension(cxx_static_assert)
#      define MOZ_STATIC_ASSERT(cond, reason)    static_assert((cond), reason)
#    endif
#  elif defined(__GNUC__)
#    if (defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L) && \
        (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))
#      define MOZ_STATIC_ASSERT(cond, reason)    static_assert((cond), reason)
#    endif
#  elif defined(_MSC_VER)
#    if _MSC_VER >= 1600 /* MSVC 10 */
#      define MOZ_STATIC_ASSERT(cond, reason)    static_assert((cond), reason)
#    endif
#  elif defined(__HP_aCC)
#    if __HP_aCC >= 62500 && defined(_HP_CXX0x_SOURCE)
#      define MOZ_STATIC_ASSERT(cond, reason)    static_assert((cond), reason)
#    endif
#  endif
#endif
#ifndef MOZ_STATIC_ASSERT
#  define MOZ_STATIC_ASSERT_GLUE1(x, y)          x##y
#  define MOZ_STATIC_ASSERT_GLUE(x, y)           MOZ_STATIC_ASSERT_GLUE1(x, y)
#  if defined(__SUNPRO_CC)
     /*
      * The Sun Studio C++ compiler is buggy when declaring, inside a function,
      * another extern'd function with an array argument whose length contains a
      * sizeof, triggering the error message "sizeof expression not accepted as
      * size of array parameter".  This bug (6688515, not public yet) would hit
      * defining moz_static_assert as a function, so we always define an extern
      * array for Sun Studio.
      *
      * We include the line number in the symbol name in a best-effort attempt
      * to avoid conflicts (see below).
      */
#    define MOZ_STATIC_ASSERT(cond, reason) \
       extern char MOZ_STATIC_ASSERT_GLUE(moz_static_assert, __LINE__)[(cond) ? 1 : -1]
#  elif defined(__COUNTER__)
     /*
      * If there was no preferred alternative, use a compiler-agnostic version.
      *
      * Note that the non-__COUNTER__ version has a bug in C++: it can't be used
      * in both |extern "C"| and normal C++ in the same translation unit.  (Alas
      * |extern "C"| isn't allowed in a function.)  The only affected compiler
      * we really care about is gcc 4.2.  For that compiler and others like it,
      * we include the line number in the function name to do the best we can to
      * avoid conflicts.  These should be rare: a conflict would require use of
      * MOZ_STATIC_ASSERT on the same line in separate files in the same
      * translation unit, *and* the uses would have to be in code with
      * different linkage, *and* the first observed use must be in C++-linkage
      * code.
      */
#    define MOZ_STATIC_ASSERT(cond, reason) \
       typedef int MOZ_STATIC_ASSERT_GLUE(moz_static_assert, __COUNTER__)[(cond) ? 1 : -1]
#  else
#    define MOZ_STATIC_ASSERT(cond, reason) \
       extern void MOZ_STATIC_ASSERT_GLUE(moz_static_assert, __LINE__)(int arg[(cond) ? 1 : -1])
#  endif
#endif

#define MOZ_STATIC_ASSERT_IF(cond, expr, reason)  MOZ_STATIC_ASSERT(!(cond) || (expr), reason)

#ifdef __cplusplus
extern "C" {
#endif

extern MFBT_API(void)
MOZ_Crash(void);

extern MFBT_API(void)
MOZ_Assert(const char* s, const char* file, int ln);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*
 * MOZ_ASSERT(expr [, explanation-string]) asserts that |expr| must be truthy in
 * debug builds.  If it is, execution continues.  Otherwise, an error message
 * including the expression and the explanation-string (if provided) is printed,
 * an attempt is made to invoke any existing debugger, and execution halts.
 * MOZ_ASSERT is fatal: no recovery is possible.  Do not assert a condition
 * which can correctly be falsy.
 *
 * The optional explanation-string, if provided, must be a string literal
 * explaining the assertion.  It is intended for use with assertions whose
 * correctness or rationale is non-obvious, and for assertions where the "real"
 * condition being tested is best described prosaically.  Don't provide an
 * explanation if it's not actually helpful.
 *
 *   // No explanation needed: pointer arguments often must not be NULL.
 *   MOZ_ASSERT(arg);
 *
 *   // An explanation can be helpful to explain exactly how we know an
 *   // assertion is valid.
 *   MOZ_ASSERT(state == WAITING_FOR_RESPONSE,
 *              "given that <thingA> and <thingB>, we must have...");
 *
 *   // Or it might disambiguate multiple identical (save for their location)
 *   // assertions of the same expression.
 *   MOZ_ASSERT(getSlot(PRIMITIVE_THIS_SLOT).isUndefined(),
 *              "we already set [[PrimitiveThis]] for this Boolean object");
 *   MOZ_ASSERT(getSlot(PRIMITIVE_THIS_SLOT).isUndefined(),
 *              "we already set [[PrimitiveThis]] for this String object");
 *
 * MOZ_ASSERT has no effect in non-debug builds.  It is designed to catch bugs
 * *only* during debugging, not "in the field".
 */
#ifdef DEBUG
   /* First the single-argument form. */
#  define MOZ_ASSERT_HELPER1(expr) \
     ((expr) ? ((void)0) : MOZ_Assert(#expr, __FILE__, __LINE__))
   /* Now the two-argument form. */
#  define MOZ_ASSERT_HELPER2(expr, explain) \
     ((expr) ? ((void)0) : MOZ_Assert(#expr " (" explain ")", __FILE__, __LINE__))
   /* And now, helper macrology up the wazoo. */
   /*
    * Count the number of arguments passed to MOZ_ASSERT, very carefully
    * tiptoeing around an MSVC bug where it improperly expands __VA_ARGS__ as a
    * single token in argument lists.  See these URLs for details:
    *
    *   http://connect.microsoft.com/VisualStudio/feedback/details/380090/variadic-macro-replacement
    *   http://cplusplus.co.il/2010/07/17/variadic-macro-to-count-number-of-arguments/#comment-644
    */
#  define MOZ_COUNT_ASSERT_ARGS_IMPL2(_1, _2, count, ...) \
     count
#  define MOZ_COUNT_ASSERT_ARGS_IMPL(args) \
	 MOZ_COUNT_ASSERT_ARGS_IMPL2 args
#  define MOZ_COUNT_ASSERT_ARGS(...) \
     MOZ_COUNT_ASSERT_ARGS_IMPL((__VA_ARGS__, 2, 1, 0))
   /* Pick the right helper macro to invoke. */
#  define MOZ_ASSERT_CHOOSE_HELPER2(count) MOZ_ASSERT_HELPER##count
#  define MOZ_ASSERT_CHOOSE_HELPER1(count) MOZ_ASSERT_CHOOSE_HELPER2(count)
#  define MOZ_ASSERT_CHOOSE_HELPER(count) MOZ_ASSERT_CHOOSE_HELPER1(count)
   /* The actual macro. */
#  define MOZ_ASSERT_GLUE(x, y) x y
#  define MOZ_ASSERT(...) \
     MOZ_ASSERT_GLUE(MOZ_ASSERT_CHOOSE_HELPER(MOZ_COUNT_ASSERT_ARGS(__VA_ARGS__)), \
                     (__VA_ARGS__))
#else
#  define MOZ_ASSERT(...) ((void)0)
#endif /* DEBUG */

/*
 * MOZ_ASSERT_IF(cond1, cond2) is equivalent to MOZ_ASSERT(cond2) if cond1 is
 * true.
 *
 *   MOZ_ASSERT_IF(isPrime(num), num == 2 || isOdd(num));
 *
 * As with MOZ_ASSERT, MOZ_ASSERT_IF has effect only in debug builds.  It is
 * designed to catch bugs during debugging, not "in the field".
 */
#ifdef DEBUG
#  define MOZ_ASSERT_IF(cond, expr)  ((cond) ? MOZ_ASSERT(expr) : ((void)0))
#else
#  define MOZ_ASSERT_IF(cond, expr)  ((void)0)
#endif

/* MOZ_NOT_REACHED_MARKER() expands (in compilers which support it) to an
 * expression which states that it is undefined behavior for the compiler to
 * reach this point. Most code should probably use the higher level
 * MOZ_NOT_REACHED (which expands to this when appropriate).
 */
#if defined(__clang__)
#  define MOZ_NOT_REACHED_MARKER() __builtin_unreachable()
#elif defined(__GNUC__)
#  if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5)
#    define MOZ_NOT_REACHED_MARKER() __builtin_unreachable()
#  endif
#elif defined(_MSC_VER)
# define MOZ_NOT_REACHED_MARKER() __assume(0)
#endif

/*
 * MOZ_NOT_REACHED(reason) indicates that the given point can't be reached
 * during execution: simply reaching that point in execution is a bug.  It takes
 * as an argument an error message indicating the reason why that point should
 * not have been reachable.
 *
 *   // ...in a language parser...
 *   void handle(BooleanLiteralNode node)
 *   {
 *     if (node.isTrue())
 *       handleTrueLiteral();
 *     else if (node.isFalse())
 *       handleFalseLiteral();
 *     else
 *       MOZ_NOT_REACHED("boolean literal that's not true or false?");
 *   }
 */
#if defined(MOZ_NOT_REACHED_MARKER)
#  if defined(DEBUG)
#    define MOZ_NOT_REACHED(reason)  do { \
                                       MOZ_Assert(reason, __FILE__, __LINE__); \
                                       MOZ_NOT_REACHED_MARKER();        \
                                     } while (0)
#  else
#    define MOZ_NOT_REACHED(reason)  MOZ_NOT_REACHED_MARKER()
#  endif
#else
#  if defined(__GNUC__)
     /*
      * On older versions of gcc we need to call a noreturn function to mark the
      * code as unreachable. Since what we want is an unreachable version of
      * MOZ_Assert, we use an asm label
      * (http://gcc.gnu.org/onlinedocs/gcc-4.6.2/gcc/Asm-Labels.html) to create
      * a new declaration to the same symbol. MOZ_ASSERT_NR should only be
      * used via this macro, as it is a very specific hack to older versions of
      * gcc.
      */
#    define MOZ_GETASMPREFIX2(X) #X
#    define MOZ_GETASMPREFIX(X) MOZ_GETASMPREFIX2(X)
#    define MOZ_ASMPREFIX MOZ_GETASMPREFIX(__USER_LABEL_PREFIX__)
     extern MOZ_NORETURN MFBT_API(void)
     MOZ_ASSERT_NR(const char* s, const char* file, int ln) \
       asm (MOZ_ASMPREFIX "MOZ_Assert");

#    define MOZ_NOT_REACHED(reason)    MOZ_ASSERT_NR(reason, __FILE__, __LINE__)
#  elif defined(DEBUG)
#    define MOZ_NOT_REACHED(reason)    MOZ_Assert(reason, __FILE__, __LINE__)
#  else
#    define MOZ_NOT_REACHED(reason)    ((void)0)
#  endif
#endif

/*
 * MOZ_ALWAYS_TRUE(expr) and MOZ_ALWAYS_FALSE(expr) always evaluate the provided
 * expression, in debug builds and in release builds both.  Then, in debug
 * builds only, the value of the expression is asserted either true or false
 * using MOZ_ASSERT.
 */
#ifdef DEBUG
#  define MOZ_ALWAYS_TRUE(expr)      MOZ_ASSERT((expr))
#  define MOZ_ALWAYS_FALSE(expr)     MOZ_ASSERT(!(expr))
#else
#  define MOZ_ALWAYS_TRUE(expr)      ((void)(expr))
#  define MOZ_ALWAYS_FALSE(expr)     ((void)(expr))
#endif

#endif /* mozilla_Assertions_h_ */
