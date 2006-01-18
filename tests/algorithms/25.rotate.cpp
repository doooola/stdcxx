/***************************************************************************
 *
 * 25.rotate.cpp - test exercising 25.2.10 [lib.alg.rotate]
 *
 * $Id$
 *
 ***************************************************************************
 *
 * Copyright (c) 1994-2006 Quovadx,  Inc., acting through its  Rogue Wave
 * Software division. Licensed under the Apache License, Version 2.0 (the
 * "License");  you may  not use this file except  in compliance with the
 * License.    You    may   obtain   a   copy   of    the   License    at
 * http://www.apache.org/licenses/LICENSE-2.0.    Unless   required    by
 * applicable law  or agreed to  in writing,  software  distributed under
 * the License is distributed on an "AS IS" BASIS,  WITHOUT WARRANTIES OR
 * CONDITIONS OF  ANY KIND, either  express or implied.  See  the License
 * for the specific language governing permissions  and limitations under
 * the License.
 * 
 **************************************************************************/

#include <algorithm>    // for rotate, rotate_copy
#include <cstring>      // for strlen
#include <cstdlib>      // for size_t, free

#include <alg_test.h>
#include <driver.h>     // for rw_test()
#include <printf.h>     // for rw_asnprintf()

_RWSTD_NAMESPACE (std) { 

// disable explicit instantiation for compilers (like MSVC)
// that can't handle it
#ifndef _RWSTD_NO_EXPLICIT_INSTANTIATION

template
void
rotate (FwdIter<assign<base<cpy_ctor> > >, 
        FwdIter<assign<base<cpy_ctor> > >, 
        FwdIter<assign<base<cpy_ctor> > >);

template
OutputIter<assign<base<cpy_ctor> > >
rotate_copy (FwdIter<assign<base<cpy_ctor> > >, 
             FwdIter<assign<base<cpy_ctor> > >,
             FwdIter<assign<base<cpy_ctor> > >, 
             OutputIter<assign<base<cpy_ctor> > >);

#endif // _RWSTD_NO_EXPLICIT_INSTANTIATION

}   // namespace std

/**************************************************************************/

// tags to select rotate or rotate_copy at compile time
struct RotateTag {
    enum { fname_inx = 0 };
    int dummy;
};

struct RotateCopyTag {
    enum { fname_inx = 1 };
    int dummy;
};

// used as a dummy template argument to test functions exercising rotate()
// (to differentiate from rotate_copy())
struct NoIterator { };
const char* type_name (NoIterator, const X*)
{
    return 0;
}


typedef unsigned char UChar;

/**************************************************************************/

const char nul_char = '\0';

template <class T>
class ToString
{
public:
    ToString (const T *first, const T *last, int pos, bool use_id = false) 
            : str_ (0) {

        std::size_t buf_sz = 0;

        if (first > last) {
            rw_asnprintf (&str_, &buf_sz, "%s", "bad range");
            return;
        }

        char* res = (char*)&nul_char;
        char* tmp = 0;

        for (const T *cur = first; cur != last; ++cur) {
            rw_asnprintf (&tmp, &buf_sz, 
                          "%s%{?}>%{;}%{?}%d:%{;}%{lc}%{?}<%{;}",
                          res,
                          cur - first == pos,    // '>'
                          use_id, cur->id_,      // "<id>:"
                          cur->val_,             // <val>
                          cur - first == pos);   // '<'

            if (res != &nul_char)
                std::free (res);

            res = tmp;
            tmp = 0;
        }

        str_ = res;
    }

    ~ToString () {
        if (str_ != &nul_char)
            std::free (str_);
    }

    operator const char* () const {
        return str_;
    }

private:

    char* str_;
};

/**************************************************************************/

unsigned iter_swap_calls;

_RWSTD_NAMESPACE (std) {

_RWSTD_SPECIALIZED_FUNCTION
void iter_swap (FwdIter<X> i, FwdIter<X> j)
{
    ++iter_swap_calls;

    const int tmp = (*i).val_;

    (*i).val_ = (*j).val_;
    (*j).val_ = tmp;
}


_RWSTD_SPECIALIZED_FUNCTION
void iter_swap (BidirIter<X> i, BidirIter<X> j)
{
    ++iter_swap_calls;

    const int tmp = (*i).val_;

    (*i).val_ = (*j).val_;
    (*j).val_ = tmp;
}


_RWSTD_SPECIALIZED_FUNCTION
void iter_swap (RandomAccessIter<X> i, RandomAccessIter<X> j)
{
    ++iter_swap_calls;

    const int tmp = (*i).val_;

    (*i).val_ = (*j).val_;
    (*j).val_ = tmp;
}

}   // namespace std

/**************************************************************************/

// exercises std::rotate()
template <class Iterator, class T>
void test_rotate (int line,
                  const char *src,
                  std::size_t midnsrc,
                  Iterator it, NoIterator, const T*, RotateTag)
{
    static const char* const itname = type_name (it, (T*)0);
    const char* const fname = "rotate";

    // compute the length of the source sequence
    const std::size_t nsrc = std::strlen (src);

    // construct a sequence of `nsrc' elements to pass to reverse
    T* const xsrc = T::from_char (src, nsrc);

    // construct iterators pointing to the beginning and end
    // of the source sequence
    const Iterator first  = make_iter (xsrc,           xsrc, xsrc + nsrc, it);
    const Iterator middle = make_iter (xsrc + midnsrc, xsrc, xsrc + nsrc, it);
    const Iterator last   = make_iter (xsrc + nsrc,    xsrc, xsrc + nsrc, it);

    // zero out the iter_swap() call counter
    iter_swap_calls = 0;

    // call rotate()
    std::rotate (first, middle, last);

    bool success = true;
    std::size_t i = 0;
    std::size_t xpos = 0;

    // verify that the sequence was correctly reversed and that it was
    // done by swapping elements i.e., not simply by copying them over
    for ( ; i != nsrc; ++i) {

        xpos = (i + (nsrc - midnsrc)) % nsrc;
        success = UChar (src [i]) == xsrc [xpos].val_;
        if (!success)
            break;
    }

    rw_assert (success, 0, line,
               "line %d: %s<%s>(\"%s\", %zu, ...) ==> %s; "
               "unexpected element value %c at %zu",
               __LINE__, fname, itname, src, midnsrc,
               (const char*) ToString<T> (xsrc, xsrc + nsrc, i),
               xsrc [xpos].val_, i);

    success = iter_swap_calls <= nsrc;
    rw_assert (success, 0, line,
               "line %d: %s<%s>(\"%s\", %zu, ...); called iter_swap() "
               "%zu times, expected no more than %zu",
               __LINE__, fname, itname, src, midnsrc,
               iter_swap_calls, nsrc);

    delete[] xsrc;
}

/**************************************************************************/

// exercises std::rotate_copy()
template <class Iterator, class OutputIterator, class T>
void test_rotate (int line,
                  const char *src,
                  std::size_t midnsrc,
                  Iterator it, OutputIterator dummy,
                  const T*, RotateCopyTag)
{
    static const char* const it1name = type_name (it, (T*)0);
    static const char* const it2name = type_name (dummy, (T*)0);
    const char* const fname = "rotate_copy";

    const std::size_t nsrc = std::strlen (src);

    T* const xsrc = T::from_char (src, nsrc);
    T* const xdst = T::from_char (src, nsrc);

    const Iterator first  = make_iter (xsrc,           xsrc, xsrc + nsrc, it);
    const Iterator middle = make_iter (xsrc + midnsrc, xsrc, xsrc + nsrc, it);
    const Iterator last   = make_iter (xsrc + nsrc,    xsrc, xsrc + nsrc, it);

    const OutputIterator result = make_iter (xdst, xdst, xdst + nsrc, dummy);

    std::size_t last_n_op_assign = T::n_total_op_assign_;

    const OutputIterator end = std::rotate_copy (first, middle, last, result);

    // verify that the returned iterator is set as expected
    bool success = end.cur_ == result.cur_ + nsrc;
    rw_assert (success, 0, line,
               "line %d: %s<%s, %s>(\"%s\", %zu, ...) == %p, "
               "got %p", __LINE__, fname, 
               it1name, it2name, src, midnsrc,
               first.cur_ + nsrc, end.cur_);

    // verify that the sequence was correctly rotated
    std::size_t i = 0;
    std::size_t xpos = 0;
    for ( ; i != nsrc; ++i) {

        xpos = (i + (nsrc - midnsrc)) % nsrc;
        success = UChar (src [i]) == xdst [xpos].val_;
        if (!success)
            break;
    }

    rw_assert (success, 0, line,
               "line %d: %s<%s, %s>(\"%s\", %zu, ...) ==> %s "
               "unexpected element value %c at %zu",
               __LINE__, fname, it1name, it2name, src, midnsrc,
               (const char*) ToString<T> (xsrc, xsrc + nsrc, i),
               xdst [xpos].val_, i);

    success = T::n_total_op_assign_ - last_n_op_assign == nsrc;
    rw_assert (success, 0, line,
               "line %d: %s<%s, %s>(\"%s\", %zu, ...); called operator=() "
               "%zu times, %zu expected",
               __LINE__, fname, it1name, it2name, src, midnsrc,
               T::n_total_op_assign_ - last_n_op_assign, nsrc);

    delete[] xsrc;
    delete[] xdst;
}

/**************************************************************************/

// exercises all two function templates
template <class Iterator1, class Iterator2, class T, class Tag>
void test_rotate (Iterator1 it1, Iterator2 it2, const T*, Tag tag)
{
    static const char* const it1name = type_name (it1, (T*)0);
    static const char* const it2name = type_name (it2, (T*)0);

    if (tag.fname_inx) {  // rotate_copy()
        rw_info (0, 0, 0, 
                 "std::rotate_copy (%s, %1$s, %1$s, %s)",
                 it1name, it2name);
    }
    else {   // reverse, rotate()
        rw_info (0, 0, 0, 
                 "std::rotate (%s, %1$s, %1$s)",
                 it1name);
    }


#define TEST(src, middle) \
    test_rotate (__LINE__, src, middle, it1, it2, (T*)0, tag)

    TEST ("",             0);
    TEST ("a",            0);
    TEST ("ab",           1);
    TEST ("abc",          1);
    TEST ("abcd",         2);
    TEST ("abcde",        2);
    TEST ("abcdef",       3);
    TEST ("abcdefg",      3);
    TEST ("abcdefgh",     4);
    TEST ("abcdefghi",    4);
    TEST ("abcdefghij",   5);

    TEST ("abcdefghijk",  0);
    TEST ("abcdefghijk",  1);
    TEST ("abcdefghijk",  2);
    TEST ("abcdefghijk",  3);
    TEST ("abcdefghijk",  4);
    TEST ("abcdefghijk",  5);
    TEST ("abcdefghijk",  6);
    TEST ("abcdefghijk",  7);
    TEST ("abcdefghijk",  8);
    TEST ("abcdefghijk",  9);
    TEST ("abcdefghijk", 10);
}

/**************************************************************************/

/* extern */ int rw_opt_no_rotate;             // --no-rotate
/* extern */ int rw_opt_no_rotate_copy;        // --no-rotate_copy
/* extern */ int rw_opt_no_output_iter;        // --no-OutputIterator
/* extern */ int rw_opt_no_fwd_iter;           // --no-ForwardIterator
/* extern */ int rw_opt_no_bidir_iter;         // --no-BidirectionalIterator
/* extern */ int rw_opt_no_rnd_iter;           // --no-RandomAccessIterator


template <class T, class Tag>
void test_rotate (const T*, Tag tag)
{
    rw_info (0, 0, 0,
             "template <class %s> std::rotate (%1$s, %1$s, %1$s)",
             "ForwardIterator");

    if (rw_opt_no_fwd_iter) {
        rw_note (0, __FILE__, __LINE__, "ForwardIterator test disabled");
    }
    else {
        test_rotate (FwdIter<T>(), NoIterator (), (T*)0, tag);
    }

    if (rw_opt_no_bidir_iter) {
        rw_note (0, __FILE__, __LINE__, "BidirectionalIterator test disabled");
    }
    else {
        test_rotate (BidirIter<T>(), NoIterator (), (T*)0, tag);
    }

    if (rw_opt_no_rnd_iter) {
        rw_note (0, __FILE__, __LINE__, "RandomAccessIterator test disabled");
    }
    else {
        test_rotate (RandomAccessIter<T>(), NoIterator (), (T*)0, tag);
    }
}

/**************************************************************************/

template <class RotateIterator, class T, class Tag>
void test_rotate_copy (RotateIterator it, const T*, Tag tag)
{
    if (rw_opt_no_output_iter) {
        rw_note (0, __FILE__, __LINE__, "OutputIterator test disabled");
    }
    else {
        test_rotate (it, OutputIter<T>(0, 0, 0), (T*)0, tag);
    }

    if (rw_opt_no_fwd_iter) {
        rw_note (0, __FILE__, __LINE__, "ForwardIterator test disabled");
    }
    else {
        test_rotate (it, FwdIter<T>(), (T*)0, tag);
    }

    if (rw_opt_no_bidir_iter) {
        rw_note (0, __FILE__, __LINE__, "BidirectionalIterator test disabled");
    }
    else {
        test_rotate (it, BidirIter<T>(), (T*)0, tag);
    }

    if (rw_opt_no_rnd_iter) {
        rw_note (0, __FILE__, __LINE__, "RandomAccessIterator test disabled");
    }
    else {
        test_rotate (it, RandomAccessIter<T>(), (T*)0, tag);
    }
}

/**************************************************************************/

template <class T, class Tag>
void test_rotate_copy(const T*, Tag tag)
{
    rw_info (0, 0, 0,
             "template <class %s, class %s> %2$s "
             "std::rotate_copy (%1$s, %1$s, %1$s, %2$s)",
             "ForwardIterator", "OutputIterator");

    if (rw_opt_no_fwd_iter) {
        rw_note (0, __FILE__, __LINE__, "ForwardIterator test disabled");
    }
    else {
        test_rotate_copy (ConstFwdIter<T>(), (T*)0, tag);
    }

    if (rw_opt_no_bidir_iter) {
        rw_note (0, __FILE__, __LINE__, "BidirectionalIterator test disabled");
    }
    else {
        test_rotate_copy (ConstBidirIter<T>(), (T*)0, tag);
    }

    if (rw_opt_no_rnd_iter) {
        rw_note (0, __FILE__, __LINE__, "RandomAccessIterator test disabled");
    }
    else {
        test_rotate_copy (ConstRandomAccessIter<T>(), (T*)0, tag);
    }
}

/**************************************************************************/

static int
run_test (int, char*[])
{
    const RotateTag     rotate_tag      = { false };
    const RotateCopyTag rotate_copy_tag = { true };

    if (rw_opt_no_rotate) {
        rw_note (0, __FILE__, __LINE__, "std::rotate test disabled");
    }
    else {
        test_rotate((X*)0, rotate_tag);
    }

    if (rw_opt_no_rotate_copy) {
        rw_note (0, __FILE__, __LINE__, "std::rotate_copy test disabled");
    }
    else {
        test_rotate_copy((X*)0, rotate_copy_tag);
    }

    return 0;
}

/**************************************************************************/

int main (int argc, char *argv[])
{
    return rw_test (argc, argv, __FILE__,
                    "lib.alg.rotate",
                    0 /* no comment */, run_test,
                    "|-no-rotate# "
                    "|-no-rotate_copy# "
                    "|-no-OutputIterator# "
                    "|-no-ForwardIterator# "
                    "|-no-BidirectionalIterator# "
                    "|-no-RandomAccessIterator#",
                    &rw_opt_no_rotate,
                    &rw_opt_no_rotate_copy,
                    &rw_opt_no_output_iter,
                    &rw_opt_no_fwd_iter,
                    &rw_opt_no_bidir_iter,
                    &rw_opt_no_rnd_iter);
}