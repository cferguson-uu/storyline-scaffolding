#ifndef MUSALIGNA7GDNETYQI385MDKWOWOO
#define MUSALIGNA7GDNETYQI385MDKWOWOO

// Author: Geert-Jan Giezeman <G.J.Giezeman@uu.nl>
// Copyright: Utrecht University, 2012-2017

#include <vector>
#include <limits>
#include "csuu_matrix.hpp"

/// \brief Namespace used for the library.
namespace musaline {

    /// \brief Namespace used for details not interesting for the user of the library.
    namespace detail {
        using Geert_cs_uu_nl::Matrix;
        const double mininf= -std::numeric_limits<double>::infinity();

        struct NwCell {
            double asb; 
            int from; 
            // 0: from topleft; 1: from top; 2: from left; 3: stop
        };

        struct NwgCell {
            double asb[3]; 
            int from; 
            bool ext_1;
            bool ext_2;
        };

    }

/**
    A Match records a match between parts of two sequences (seq1 and seq2).
*/
    struct Match {
    /** \brief n[0] and n[1] hold the number of elements of seq1 and seq2 that
        are involved. At most one of them can be 0.
    */
        int n[2];
        /// The contribution of this match to the total alignment score.
        double score;
    };

/**
 * An Alignment records how two sequences (seq1 and seq2) are aligned.

 * Some invariants hold between the matches and the other values.
 * - The score is the sum of all matches[i].score
 * - end[j]-start[j] is the sum of all matches[i].n[j], for j equal to 0 or 1.
 * .
*/
    struct Alignment {
    /** \brief The computed similarity value for the alignment.
     */
        double score;
    /**
      \brief start[i] points to the start of the matched part of sequence i.

        start[i] together with end[i]
        indicates the subsequence that was matched of sequence i.
        start indicates the first element.
        start[i]<=end[i], and end[i]-start[i] is the length of the
        subsequence that is matched.
    */
        size_t start[2];
    /**
      \brief end[i] points one after the end of the matched part of sequence i.
    */
        size_t  end[2];
        /**
    \brief This vector holds details of the parts of seq1 and seq2
    that are matched with each other.
    */
        std::vector< Match > matches;
    };


#if 0
    /**
      \brief Enumeration type to record the different alignment possibilities.

     The values 0 and 1 for FirstSeqWithGap and SecondSeqWithGap can be
     relied upon.
      */
    enum AlignKind {FirstSeqWithGap=0, SecondSeqWithGap=1, BothSeqs, FirstWithMany, SecondWithMany};

/**
    An Alignment records how two sequences (seq1 and seq2) are aligned.

*/
    struct Alignment {
    /// \brief The computed similarity value for the alignment.
        double score;
    /**
        start[i] together with end[i]
        indicates the subsequence that was matched of sequence i.
        start indicates the first element.
        start[i]<=end[i], and end[i]-start[i] is the length of the
        subsequence that is matched.
    */
        size_t start[2];
    /**
      end[i] points one after the end of sequence i.
    */
        size_t  end[2];
        /**
    The vector 'a' holds the alignment types: an element of a sequence can
    align with an element of the other sequence (BothSeqs) or with a
    gap (FirstSeqWithGap and SecondSeqWithGap).
    */
        std::vector< AlignKind > a;
    };
#endif

    /**
    \brief This class can compute the most simple form of alignment.

    An element of a sequence can be aligned with an element of the other
    sequence or with a gap. The cost of aligning two elements or an element
    with a gap, does not depend on the context.

    The class has several align functions which differ in what part of the
    sequences should be aligned. Details are provided with the individual
    align functions.

    This class takes a CostCalculator class as a template parameter.
    This class shall be provided by the user and gives the benefit of
    matching one element with another one. If you can call the following
    function template with your cost calculator and sequences, you should
    be OK. More detailed requirements are listed after the function.

\code
#include "boost/range.hpp"

    template <class CostCalculator, class Sequence1, class Sequence2>
    void linear_usage(CostCalculator ccal,
            Sequence1 const &seq1, Sequence2 const &seq2)
    {
        // Copy constructor
        CostCalculator ccal_copy(ccal);
        // assignment operator
        ccal_copy = ccal;
        ccal.preprocess(seq1, seq2);
        double d = ccal.gap_bft();
        size_t sz1 = boost::distance(seq1);
        size_t sz2 = boost::distance(seq2);
        auto s1cur = boost::const_begin(seq1);
        for (size_t i=0; i!=sz1; ++i) {
            auto s2cur = boost::const_begin(seq2);
            for (size_t j=0; j!=sz2; ++j) {
                double benefit = ccal.match(*s1cur, *s2cur);
                ++s2cur;
            }
            ++s1cur;
        }
    }
\endcode

   The classes Sequence1 and Sequence2 should fulfill the requirements for
   a boost Forward Range. See the documentation of the range library on
   http://www.boost.org for full details. A range can be thought of as a
   'container light'. It should give access to a begin and end iterator, which
   should be forward iterators.

   The CostCalculator class should have the following member functions:
   \li a copy constructor and an assignment operator.
   \li \c match which
   takes an element of seq1 and an element of seq2 and returns a
   double. This value is a measure for how well the two elements match. A
   good match should yield a (big) positive value. A bad match should yield
   a negative value.
   \li \c gap_bft. Returns the benefit for matching an element with a gap
   (nothing). The returned value should be negative, otherwise, the results
   are not guaranteed to be the optimal solution.
   \li \c preprocess which takes two sequences as argument. This member
   function is guaranteed to be called before any call to \c match for those
   sequences.
   This may be used to do some preprocessing of the sequences. For
   instance, say we want to compare two melodies, where a melody is a
   sequences of notes and a key. If one of the melodies is in C and the
   other one is in G, we might want to transpose the latter melody. This
   can be done in the preprocess step.

   In \ref sec_lin_require "introductory page" you can find a more gentle
   (and less precise) description of the requirements.

     */
    template <class CostCalc>
    class LinearAligner {
    public:
        typedef CostCalc CostCalculator;
    private:
        mutable CostCalculator m_ccal;
        template <class Iter1, class Iter2>
        void fill_matrix_std(Geert_cs_uu_nl::Matrix<detail::NwCell> &matrix,
                Iter1 s1cur, Iter2 s2begin) const;
        template <class Iter1, class Iter2>
        void fill_matrix_partial(Geert_cs_uu_nl::Matrix<detail::NwCell> &matrix,
                Iter1 s1cur, Iter2 s2begin) const;
    public:
        /** \brief Constructor. 

        */
        LinearAligner(
                CostCalculator ccal = CostCalculator())
        :
        m_ccal(ccal) {}
        /// Set a different CostCalculator object.
        void set_cost_calculator(CostCalculator ccal) { m_ccal = ccal;}
        /// Get a const reference to the current CostCalculator object.
        CostCalculator const &get_cost_calculator() const { return m_ccal;}
        ///  align both complete sequences.
        ///  The resulting Alignment will have
        ///  start[i]==0 and end[i]==size of seq i, for i==1 and 2.
        template <class Sequence1, class Sequence2>
        Alignment global_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// align start of first sequence with the complete second sequence.
        /// start[0]==0, start[1]==0 and 
        /// end[1]==size of seq2.
        template <class Sequence1, class Sequence2>
        Alignment cut_first_end_off_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// align complete first sequence with the start of second sequence.
        /// start[0]==0, start[1]==0 and 
        /// end[0]==size of seq1.
        template <class Sequence1, class Sequence2>
        Alignment cut_second_end_off_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// align one complete sequence with the start of the other sequence.
        /// start[0]==0, start[1]==0 and either
        /// end[0]==size of seq1 or end[1]==size of seq2.
        template <class Sequence1, class Sequence2>
        Alignment cut_one_end_off_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// align one complete sequence with the end of the other sequence.
        /// end[0]==size of seq1, end[1]==size of seq2 and
        /// either start[0]==0 or start[1]==0.
        template <class Sequence1, class Sequence2>
        Alignment cut_one_begin_off_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// align the start of the sequences.
        /// start[0]==0 and start[1]==0.
        template <class Sequence1, class Sequence2>
        Alignment initial_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// align the end of the sequences.
        /// end[0]==size of seq1 and end[1]==size of seq2.
        template <class Sequence1, class Sequence2>
        Alignment end_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// align seq1 inside seq2. start[0]==0 and
        /// end[0]==size of sequence 0.
        template <class Sequence1, class Sequence2>
        Alignment inside_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// start[0]==0 or start[1]==0
        /// end[0]==size of seq1 or end[1]==size of seq2.
        template <class Sequence1, class Sequence2>
        Alignment semi_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// No requirement for start and end.
        template <class Sequence1, class Sequence2>
        Alignment partial_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
    };

     template <typename CostCalculator>
     LinearAligner<CostCalculator> make_linear_aligner(CostCalculator const &ccal)
     {
         return LinearAligner<CostCalculator>(ccal);
     }


    /**
    \brief This class is almost the same as LinearAligner. Only the requirements
    for the cost calculator are different, allowing more freedom in computing
    the matching with a gap.

    This class takes a CostCalculator class as a template parameter.
    This class shall be provided by the user and gives the benefit of
    matching one element with another one. If you can call the following
    function template with your cost calculator and sequences, you should
    be OK. More detailed requirements are listed after the function.

\code
#include "boost/range.hpp"

    template <class CostCalculator, class Sequence1, class Sequence2>
    void linear_usage(CostCalculator const &ccal,
            Sequence1 const &seq1, Sequence2 const &seq2)
    {
        // Copy constructor
        CostCalculator ccal_copy(ccal);
        // assignment operator
        ccal_copy = ccal;
        ccal.preprocess(seq1, seq2);
        size_t sz1 = boost::distance(seq1);
        size_t sz2 = boost::distance(seq2);
        auto s1cur = boost::const_begin(seq1);
        double d = ccal.match_gap_with1(*cur1);
        for (size_t i=0; i!=sz1; ++i) {
            auto s2cur = boost::const_begin(seq2));
            d = ccal.match_gap_with2(*cur2);
            for (size_t j=0; j!=sz2; ++j) {
                double benefit = ccal.match(*s1cur, *s2cur);
                ++s2cur;
            }
            ++s1cur;
        }
    }
\endcode

   The classes Sequence1 and Sequence2 should fulfill the requirements for
   a boost Forward Range. See the documentation of the range library on
   http://www.boost.org for full details. A range can be thought of as a
   'container light'. It should give access to a begin and end iterator, which
   should be forward iterators.

   The CostCalculator class should have the following member functions:
   \li a copy constructor and an assignment operator.
   \li \c match which
   takes an element of seq1 and an element of seq2 and returns a
   double. This value is a measure for how well the two elements match. A
   good match should yield a (big) positive value. A bad match should yield
   a negative value.
   \li \c match_gap_with1. Returns the benefit for matching an element of
   sequence 1 with a gap (nothing). The returned value should be negative,
   otherwise, the results are not guaranteed to be the optimal solution.
   \li \c match_gap_with2. Returns the benefit for matching an element of
   sequence 2 with a gap (nothing). The returned value should be negative,
   otherwise, the results are not guaranteed to be the optimal solution.
   \li \c preprocess which takes two sequences as argument. This member
   function is guaranteed to be called before any call to \c match for those
   sequences.
   This may be used to do some preprocessing of the sequences. For
   instance, say we want to compare two melodies, where a melody is a
   sequences of notes and a key. If one of the melodies is in C and the
   other one is in G, we might want to transpose the latter melody. This
   can be done in the preprocess step.

   In \ref sec_lin_require "introductory page" you can find a more gentle
   (and less precise) description of the requirements.

     */
    template <class CostCalc>
    class LinearAligner2 {
    public:
        typedef CostCalc CostCalculator;
    private:
        mutable CostCalculator m_ccal;
        template <class Sequence2>
        void gap_init_first_column(
                Geert_cs_uu_nl::Matrix<detail::NwCell> &matrix,
                Sequence2 const &seq2) const;
        template <class Sequence1>
        void gap_init_first_row(
                Geert_cs_uu_nl::Matrix<detail::NwCell> &matrix,
                Sequence1 const &seq1) const;
        template <class Sequence0, class Sequence1>
        void gap_init(
                Geert_cs_uu_nl::Matrix<detail::NwCell> &matrix,
                Sequence0 const &seq0, Sequence1 const &seq1) const;
        template <class Iter1, class Iter2>
        void fill_matrix_std(Geert_cs_uu_nl::Matrix<detail::NwCell> &matrix,
                Iter1 s1cur, Iter2 s2begin) const;
        template <class Iter1, class Iter2>
        void fill_matrix_partial(Geert_cs_uu_nl::Matrix<detail::NwCell> &matrix,
                Iter1 s1cur, Iter2 s2begin) const;
    public:
        /** \brief Constructor. 

        */
        LinearAligner2(
                CostCalculator ccal = CostCalculator())
        :
        m_ccal(ccal) {}
        /// Set a different CostCalculator object.
        void set_cost_calculator(CostCalculator ccal) { m_ccal = ccal;}
        /// Get a const reference to the current CostCalculator object.
        CostCalculator const &get_cost_calculator() const { return m_ccal;}
        ///  align both complete sequences.
        ///  The resulting Alignment will have
        ///  start[i]==0 and end[i]==size of seq i, for i==1 and 2.
        template <class Sequence1, class Sequence2>
        Alignment global_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// align start of first sequence with the complete second sequence.
        /// start[0]==0, start[1]==0 and 
        /// end[1]==size of seq2.
        template <class Sequence1, class Sequence2>
        Alignment cut_first_end_off_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// align complete first sequence with the start of second sequence.
        /// start[0]==0, start[1]==0 and 
        /// end[0]==size of seq1.
        template <class Sequence1, class Sequence2>
        Alignment cut_second_end_off_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// align one complete sequence with the start of the other sequence.
        /// start[0]==0, start[1]==0 and either
        /// end[0]==size of seq1 or end[1]==size of seq2.
        template <class Sequence1, class Sequence2>
        Alignment cut_one_end_off_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// align one complete sequence with the end of the other sequence.
        /// end[0]==size of seq1, end[1]==size of seq2 and
        /// either start[0]==0 or start[1]==0.
        template <class Sequence1, class Sequence2>
        Alignment cut_one_begin_off_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// align the start of the sequences.
        /// start[0]==0 and start[1]==0.
        template <class Sequence1, class Sequence2>
        Alignment initial_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// align the end of the sequences.
        /// end[0]==size of seq1 and end[1]==size of seq2.
        template <class Sequence1, class Sequence2>
        Alignment end_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// align seq1 inside seq2. start[0]==0 and
        /// end[0]==size of sequence 0.
        template <class Sequence1, class Sequence2>
        Alignment inside_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// start[0]==0 or start[1]==0
        /// end[0]==size of seq1 or end[1]==size of seq2.
        template <class Sequence1, class Sequence2>
        Alignment semi_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// No requirement for start and end.
        template <class Sequence1, class Sequence2>
        Alignment partial_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
    };

     template <typename CostCalculator>
     LinearAligner2<CostCalculator> make_linear_aligner(CostCalculator const &ccal)
     {
         return LinearAligner2<CostCalculator>(ccal);
     }



    /**
    \brief This class can compute a form of alignment with a slightly
    more complicated cost function than LinearAligner.

    An element of a sequence can be aligned with an element of the other
    sequence or with a gap. The cost of aligning two elements or an element
    does not depend on the context, but the cost of extending a gap can be
    different from starting a gap.

    The class has several align functions which differ in what part of the
    sequences should be aligned. Details are provided with the individual
    align functions.

    This class takes a CostCalculator class as a template parameter.
    This class shall be provided by the user and gives the benefit of
    matching one element with another one. If you can call the following
    function template with your cost calculator and sequences, you should
    be OK. More detailed requirements are listed after the function.

\code
#include "boost/range.hpp"

    template <class CostCalculator, class Sequence1, class Sequence2>
    void affine_usage(CostCalculator const &ccal,
            Sequence1 const &seq1, Sequence2 const &seq2)
    {
        // Copy constructor
        CostCalculator ccal_copy(ccal);
        // assignment operator
        ccal_copy = ccal;
        ccal.preprocess(seq1, seq2);
        double d = ccal.gap_bft();
        d = ccal.ext_bft();
        size_t sz1 = boost::distance(seq1);
        size_t sz2 = boost::distance(seq2);
        auto s1cur = boost::const_begin(seq1);
        for (size_t i=0; i!=sz1; ++i) {
            auto s2cur = boost::const_begin(seq2));
            for (size_t j=0; j!=sz2; ++j) {
                double benefit = ccal.match(*s1cur, *s2cur);
                ++s2cur;
            }
            ++s1cur;
        }
    }
\endcode

   The classes Sequence1 and Sequence2 should fulfill the requirements for
   a boost Forward Range. See the documentation of the range library on
   http://www.boost.org for full details. A range can be thought of as a
   'container light'. It should give access to a begin and end iterator, which
   should be forward iterators.

   The CostCalculator class has the same requirements as the CostCalculator
   for the LinearAligner class, plus one extra required member function:
   \li \c ext_bft. Returns the benefit for extending a gap, that is, for
   matching an element with a gap, where the previous element was also matched
   with a gap.
   The returned value should be negative, otherwise, the results
   are not guaranteed to be the optimal solution.

     */
    template <class CostCalc>
    class AffineAligner {
    public:
        typedef CostCalc CostCalculator;
    private:
        mutable CostCalculator m_ccal;

        template <class Iter1, class Iter2>
        void fill_matrix_std(Geert_cs_uu_nl::Matrix<detail::NwgCell> &matrix,
                Iter1 s1cur, Iter2 s2begin) const;
        template <class Iter1, class Iter2>
        void fill_matrix_partial(Geert_cs_uu_nl::Matrix<detail::NwgCell> &matrix,
                Iter1 s1cur, Iter2 s2begin) const;
    public:
        /** \brief Constructor. 

        */
        AffineAligner( CostCalculator ccal = CostCalculator())
        : m_ccal(ccal)
        {}
        /// Set a different CostCalculator object.
        void set_cost_calculator(CostCalculator ccal) { m_ccal = ccal;}
        /// Get a const reference to the current CostCalculator object.
        CostCalculator const &get_cost_calculator() const { return m_ccal;}
        ///  align both complete sequences.
        ///  The resulting Alignment will have
        ///  start[i]==0 and end[i]==size of seq i, for i==1 and 2.
        template <class Sequence1, class Sequence2>
        Alignment global_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// align start of first sequence with the complete second sequence.
        /// start[0]==0, start[1]==0 and 
        /// end[1]==size of seq2.
        template <class Sequence1, class Sequence2>
        Alignment cut_first_end_off_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// align complete first sequence with the start of second sequence.
        /// start[0]==0, start[1]==0 and 
        /// end[0]==size of seq1.
        template <class Sequence1, class Sequence2>
        Alignment cut_second_end_off_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// align one complete sequence with the start of the other sequence.
        /// start[0]==0, start[1]==0 and either
        /// end[0]==size of seq1 or end[1]==size of seq2.
        template <class Sequence1, class Sequence2>
        Alignment cut_one_end_off_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// align one complete sequence with the end of the other sequence.
        /// end[0]==size of seq1, end[1]==size of seq2 and
        /// either start[0]==0 or start[1]==0.
        template <class Sequence1, class Sequence2>
        Alignment cut_one_begin_off_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// align seq1 inside seq2. start[0]==0 and
        /// end[0]==size of sequence 0.
        template <class Sequence1, class Sequence2>
        Alignment inside_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// align the start of the sequences.
        /// start[0]==0 and start[1]==0.
        template <class Sequence1, class Sequence2>
        Alignment initial_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// align the end of the sequences.
        /// end[0]==size of seq1 and end[1]==size of seq2.
        template <class Sequence1, class Sequence2>
        Alignment end_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// start[0]==0 or start[1]==0
        /// end[0]==size of seq1 or end[1]==size of seq2.
        template <class Sequence1, class Sequence2>
        Alignment semi_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// No requirement for start and end.
        template <class Sequence1, class Sequence2>
        Alignment partial_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
    };

     template <typename CostCalculator>
     AffineAligner<CostCalculator> make_affine_aligner(CostCalculator const &ccal)
     {
         return AffineAligner<CostCalculator>(ccal);
     }

#if 0

    /**

      */
    struct AlignType2 {
        AlignKind ak;
        /**
         * The score for aligning those elements.
         */
        double score;
        /** In the case that \c ak is FirstWithMany,
         * the number of symbols of the second sequence that are matched to
         * one symbol of the first sequence.
         * Vice versa in the SecondWithMany case. Otherwise undefined.
        */
        int n;
    };

/**
    An Alignment2 records how two sequences (seq1 and seq2) are aligned, when fragmentation and consolidation is allowed.

*/
    struct Alignment2 {
    /// \brief The computed similarity value for the alignment.
        double score;
    /**
        start[i] together with end[i]
        indicates the subsequence that was matched of sequence i.
        start indicates the first element.
        start[i]<=end[i], and end[i]-start[i] is the length of the
        subsequence that is matched.
    */
        size_t start[2];
    /**
      end[i] points one after the end of sequence i.
    */
        size_t  end[2];
        /**
    The vector 'a' holds the alignment types: an element of a sequence can
    align with an element of the other sequence (BothSeqs) or with a
    gap (FirstSeqWithGap and SecondSeqWithGap) or with multiple elements
    (FirstWithMany and SecondWithMany).
    */
        std::vector<AlignType2> a;
    };
#endif

    namespace detail {
        struct FrCell {
            double asb; 
            int from; 
            int fragments;
        };
    }

    /**
    An element of a sequence can be aligned with an element of the other
    sequence or with a gap. The cost of aligning two elements or an element
    with a gap, does not depend on the context.

    The class has several align functions which differ in what part of the
    sequences should be aligned. Details are provided with the individual
    align functions.

    This class takes a CostCalculator class as a template parameter.  The
    requirements are much like those for LinearAligner, except that \c
    gap_bft is replaced by \c match_gap_with1 and \c match_gap_with2, which
    take an element as parameter. Furthermore, there should be member
    functions \c match_range_with1 and \c match_range_with2. 

    If you can call the following function
    template with your cost calculator and sequences without compilation
    errors, you should be OK. More detailed requirements are listed after
    the function.

\code
#include "boost/range.hpp"

    template <class CostCalculator, class Sequence1, class Sequence2>
    void consolidating_usage(CostCalculator const &ccal,
            Sequence1 const &seq1, Sequence2 const &seq2)
    {
        // Copy constructor
        CostCalculator ccal_copy(ccal);
        // assignment operator
        ccal_copy = ccal;
        ccal.preprocess(seq1, seq2);
        size_t sz1 = boost::distance(seq1);
        size_t sz2 = boost::distance(seq2);
        auto s1begin = boost::const_begin(seq1);
        auto s1cur(s1begin)
        for (size_t i=0; i!=sz1; ++i) {
            auto s2begin = boost::const_begin(seq2));
            auto s2cur = s2begin;
            double d = ccal.match_gap_with1(*s1cur);
            for (size_t j=0; j!=sz2; ++j) {
                double benefit = ccal.match(*s1cur, *s2cur);
                d = ccal.match_gap_with2(*s2cur);
                vector<double> benefits = ccal.match_range_with1(*s1cur,
                    s2begin, s2cur);
                benefits = ccal.match_range_with2(s1begin, s1cur, *s2cur);
                ++s2cur;
            }
            ++s1cur;
        }
    }
\endcode
*/

    /**
    The classes Sequence1 and Sequence2 must fulfill the requirements for
    a boost Bidirectional Range. See the documentation of the range library on
    http://www.boost.org for full details. A range can be thought of as a
    'container light'. It should give access to a begin and end iterator, which
    should be bidirectional iterators.

    The CostCalculator class should have the following member functions:
    \li a copy constructor and an assignment operator.
    \li \c match which
    takes an element of seq1 and an element of seq2 and returns a
    double. This value is a measure for how well the two elements match. A
    good match should yield a (big) positive value. A bad match should yield
    a negative value.
    \li \c match_gap_with1. Takes an element of the first sequence as parameter.
    Returns the benefit for matching that element with a gap (nothing).
    The returned value should be negative, otherwise, the results
    are not guaranteed to be the optimal solution.
    \li \c match_gap_with2. Takes an element of the second sequence as parameter.
    Returns the benefit for matching that element with a gap (nothing).
    \li \c match_range_with1.
    Takes three parameters. The first is an element of the first sequence. The
    second and third are the begin and end iterator of a range of the
    second sequences. The result should be a vector of doubles.
    The resulting vector may have any length less than the size of the
    supplied range. The first element (if existant) contains the benefit of
    matching the last two elements of the range with the first parameter
    element. The second element of the result contains the benefit of
    matching the last three elements of the range with the first parameter
    element, etcetera.
    \li \c match_range_with2. Matches an element of the second sequence with a
     number of elements of the first sequence.
    \li \c preprocess which takes two sequences as argument. This member
    function is guaranteed to be called before any call to the match functions
    for those sequences.
    This may be used to do some preprocessing of the sequences. For
    instance, say we want to compare two melodies, where a melody is a
    sequences of notes and a key. If one of the melodies is in C and the
    other one is in G, we might want to transpose the latter melody. This
    can be done in the preprocess step.

    In \ref sec_consol_require "introductory page" you can find a more gentle
    (and less precise) description of the requirements.

    */
    template <class CostCalc>
    class ConsolidatingAligner {
    public:
        typedef CostCalc CostCalculator;
    private:
        mutable CostCalculator m_ccal;
        template <class Iter1, class Iter2>
        void fill_matrix(Geert_cs_uu_nl::Matrix<detail::FrCell> &matrix,
                Iter1 s1begin, Iter2 s2begin, double initial_value) const;
        template <class Iter1, class Iter2>
        void fill_matrix_std(Geert_cs_uu_nl::Matrix<detail::FrCell> &matrix,
                Iter1 s1begin, Iter2 s2begin) const
        {
            fill_matrix(matrix, s1begin, s2begin, detail::mininf);
        }
        template <class Iter1, class Iter2>
        void fill_matrix_partial(Geert_cs_uu_nl::Matrix<detail::FrCell> &matrix,
                Iter1 s1begin, Iter2 s2begin) const
        {
            fill_matrix(matrix, s1begin, s2begin, 0.0);
        }

        template <class Sequence2>
        void gap_init_first_column(
                Geert_cs_uu_nl::Matrix<detail::FrCell> &matrix,
                Sequence2 const &seq2) const;
        template <class Sequence1>
        void gap_init_first_row(Geert_cs_uu_nl::Matrix<detail::FrCell> &matrix,
            Sequence1 const &seq1) const;

        template <class Sequence1, class Sequence2>
        void gap_init(Geert_cs_uu_nl::Matrix<detail::FrCell> &matrix,
            Sequence1 const &seq1, Sequence2 const &seq2) const;

    public:
        /// \brief Constructor. Takes a CostaCalculator object as
        /// parameter.
        ConsolidatingAligner(
                CostCalculator ccal = CostCalculator())
        : m_ccal(ccal) {}
        /// Set a different CostCalculator object.
        void set_cost_calculator(CostCalculator ccal) { m_ccal = ccal;}
        /// Get a const reference to the current CostCalculator object.
        CostCalculator const &get_cost_calculator() const { return m_ccal;}
        ///  align both complete sequences.
        ///  The resulting Alignment will have
        ///  start[i]==0 and end[i]==size of seq i, for i==1 and 2.
        template <class Sequence1, class Sequence2>
        Alignment global_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// align start of first sequence with the complete second sequence.
        /// start[0]==0, start[1]==0 and 
        /// end[1]==size of seq2.
        template <class Sequence1, class Sequence2>
        Alignment cut_first_end_off_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// align start of first sequence with the complete second sequence.
        /// start[0]==0, start[1]==0 and 
        /// end[1]==size of seq2.
        template <class Sequence1, class Sequence2>
        Alignment cut_second_end_off_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// align one complete sequence with the start of the other sequence.
        /// start[0]==0, start[1]==0 and either
        /// end[0]==size of seq1 or end[1]==size of seq2.
        template <class Sequence1, class Sequence2>
        Alignment cut_one_end_off_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// align one complete sequence with the end of the other sequence.
        /// end[0]==size of seq1, end[1]==size of seq2 and
        /// either start[0]==0 or start[1]==0.
        template <class Sequence1, class Sequence2>
        Alignment cut_one_begin_off_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// align the start of the sequences.
        /// start[0]==0 and start[1]==0.
        template <class Sequence1, class Sequence2>
        Alignment initial_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// align the end of the sequences.
        /// end[0]==size of seq1 and end[1]==size of seq2.
        template <class Sequence1, class Sequence2>
        Alignment end_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// align seq1 inside seq2. start[0]==0 and
        /// end[0]==size of sequence 0.
        template <class Sequence1, class Sequence2>
        Alignment inside_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// start[0]==0 or start[1]==0
        /// end[0]==size of seq1 or end[1]==size of seq2.
        template <class Sequence1, class Sequence2>
        Alignment semi_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
        /// No requirement for start and end.
        template <class Sequence1, class Sequence2>
        Alignment partial_align(Sequence1 const &seq1, Sequence2 const &seq2) const;
    };

     template <typename CostCalculator>
     ConsolidatingAligner<CostCalculator> make_consolidating_aligner(CostCalculator const &ccal)
     {
         return ConsolidatingAligner<CostCalculator>(ccal);
     }

}

#include "musaline_impl.hpp"
#include "musaline_fragment.hpp"

// vim:set shiftwidth=4 softtabstop=4 expandtab cindent:

#endif // MUSALIGNA7GDNETYQI385MDKWOWOO
