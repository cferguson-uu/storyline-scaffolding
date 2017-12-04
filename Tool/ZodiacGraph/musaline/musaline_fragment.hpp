#ifndef MUSALIGNA7GDNETYQI385MDKWOWOOA
#define MUSALIGNA7GDNETYQI385MDKWOWOOA

// Author: Geert-Jan Giezeman <G.J.Giezeman@uu.nl>
// Copyright: Utrecht University, 2012-2017

#include <vector>
#include <cassert>
#include "csuu_matrix.hpp"
#include "boost/range.hpp"

/*
 * ConsolidatingAligner (Needleman, Wunsch)
 * We compute the best alignment by filling in a matrix. We fill in
 * cumulative benefits for matching elements of sequences 0 and 1.
 * The matrix holds one column for every element of sequence 0, plus one
 * (for border condition).
 * The matrix holds one row for every element of sequence 1, plus one
 * (for border condition).
 * The matrix element (r+1,c+1) holds the best way of matching sequence 0 up
 * to and including c with sequence 1 up to and including r.
 * We maintain a score value asb, the accumulated similarity benefit.
 * We also record a value 'from', where we record what the previous position was
 * - from==0 : align elements c and r of the sequences 0 and 1
 * - from==1 : align element c of sequence 0 with a gap
 * - from==2 : align element r of sequence 1 with a gap
 * - from==3 : start alignment here
 * - from==4 : align asb.fragments elements of sequence 0 with 1 of sequence 1
 * - from==5 : align asb.fragments elements of sequence 1 with 1 of sequence 0
 */

namespace musaline {

    namespace detail {

        inline void set(FrCell &cell, double a, int fr)
        {
            assert(0<=fr && fr<4);
            cell.asb=a;
            cell.from=fr;
            cell.fragments=1;
        }

        inline void set(FrCell &cell, double a, int from, int fragments)
        {
            assert(4<=from && from<6);
            cell.asb=a;
            cell.from=from;
            cell.fragments=fragments;
        }

    }


    namespace detail {

    void zero_init_first_column(Matrix<FrCell> &matrix)
    {
        for (size_t i=1; i<matrix.row_size();++i) {
            set(matrix(i,0), 0.0, 3);
        }
    }
    void zero_init_first_row(Matrix<FrCell> &matrix)
    {
        for (size_t i=1; i<matrix.column_size();++i) {
            set(matrix(0,i), 0.0, 3);
        }
    }
    void zero_init(Matrix<FrCell> &matrix)
    {
        set(matrix(0, 0), 0.0, 3);
        zero_init_first_row(matrix);
        zero_init_first_column(matrix);
    }

    void find_better_in_column(double &best, size_t&cur_r, size_t&cur_c, 
            Matrix<FrCell> const &matrix)
    {
        size_t c = matrix.column_size()-1;
        for (size_t r=0; r<matrix.row_size();++r) {
            FrCell const&cell = matrix(r,c);
            if (cell.asb>best) {
                best=cell.asb;
                cur_r=r;
                cur_c=c;
            }
        }
    }

    void find_better_in_row(double &best, size_t&cur_r, size_t&cur_c, 
            Matrix<FrCell> const &matrix)
    {
        size_t r = matrix.row_size()-1;
        for (size_t c=0; c<matrix.column_size();++c) {
            FrCell const&cell = matrix(r,c);
            if (cell.asb>best) {
                best=cell.asb;
                cur_r=r;
                cur_c=c;
            }
        }
    }
    void find_best(double &best, size_t &cur_r, size_t &cur_c, 
            Matrix<FrCell> const &matrix)
    {
        best = 0.0;
        cur_r=0;
        cur_c=0;
        for (size_t r=1; r<matrix.row_size();++r) {
            for (size_t c=1; c<matrix.column_size();++c) {
                FrCell const&cell = matrix(r,c);
                if (cell.asb>best) {
                    best=cell.asb;
                    cur_r=r;
                    cur_c=c;
                }
            }
        }
    }

    Alignment trace_back(Matrix<FrCell> &matrix,
        size_t cur_r, size_t cur_c)
    {
        Alignment result;
        result.score = matrix(cur_r, cur_c).asb;
        result.end[0]=cur_c;
        result.end[1]=cur_r;
        std::vector<Match> al;
        al.reserve(cur_c+cur_r);
        bool tracking=true;
        //AlignType2 at;
        FrCell const *cur = &matrix(cur_r, cur_c);
        double to_score = cur->asb;
        while (true) {
            // check which way to go
            Match match;
            switch (cur->from) {
            case 0:
                match.n[0]=1;
                match.n[1]=1;
                --cur_c;
                --cur_r;
                break;
            case 1:
                match.n[0]=1;
                match.n[1]=0;
                --cur_c;
                break;
            case 2:
                match.n[0]=0;
                match.n[1]=1;
                --cur_r;
                break;
            case 3:
                tracking=false;
                break;
            case 4:
                match.n[0]=1;
                match.n[1]=cur->fragments;
                --cur_c;
                cur_r -= cur->fragments;
                break;
            case 5:
                match.n[0]=cur->fragments;
                match.n[1]=1;
                cur_c -= cur->fragments;
                --cur_r;
                break;
            default:
                assert(false);
                throw std::logic_error("");
            }
            if (!tracking)
                break;
            cur = &matrix(cur_r, cur_c);
            double from_score=cur->asb;
            match.score = to_score-from_score;
            al.push_back(match);
            to_score=from_score;
        }
        result.start[0]=cur_c;
        result.start[1]=cur_r;
        std::reverse_copy(al.begin(), al.end(),
                std::back_inserter(result.matches));
        return result;
    }

    void fill_cell(FrCell &cur, FrCell const &tl,
            FrCell const &l, FrCell const &t,
            double match_bft, double col_gap_bft, double row_gap_bft)
    {
        // compute accumulated similarity benefit of matching
        // column c with row r
        double rd_bft = match_bft + tl.asb;
        if (rd_bft > cur.asb) {
            cur.asb = rd_bft;
            cur.from = 0;
            cur.fragments = 1;
        }
        // compute accumulated similarity benefit of matching column c with gap
        double right_bft = col_gap_bft + l.asb;
        if (right_bft > cur.asb) {
            cur.asb = right_bft;
            cur.from = 1;
            cur.fragments = 1;
        }
        // compute accumulated similarity benefit of matching row r with gap
        double down_bft = row_gap_bft + t.asb;
        if (down_bft > cur.asb) {
            cur.asb = down_bft;
            cur.from = 2;
            cur.fragments = 1;
        }
    }
    }

    template <class CostCalculator>
    template <class Iter1, class Iter2>
    void ConsolidatingAligner<CostCalculator>::fill_matrix(
            Geert_cs_uu_nl::Matrix<detail::FrCell> &matrix,
            Iter1 s1begin, Iter2 s2begin, double initial_value) const
    {
        using namespace detail;
        Iter1 s1cur(s1begin);
        for (size_t c=1; c<matrix.column_size(); ++c) {
            typename std::iterator_traits<Iter1>::value_type n1 = *s1cur;
            ++s1cur;
            Iter2 s2cur(s2begin);
            for (size_t r=1; r<matrix.row_size(); ++r) {
                typename std::iterator_traits<Iter2>::value_type n2 = *s2cur;
                ++s2cur;
                FrCell &cur = matrix(r, c);
                set(cur, initial_value, 3);
                fill_cell(cur, matrix(r-1, c-1), matrix(r, c-1),
                        matrix(r-1, c), m_ccal.match(n1, n2),
                        m_ccal.match_gap_with1(n1), m_ccal.match_gap_with2(n2));
                // fragmentation and consolidation
                // That is, try to match 1 note of Seq0 with multiple of Seq1
                // and vice versa.
                std::vector<double> benefits1;
                if (r>1) {
                    benefits1 = m_ccal.match_range_with1(n1, s2begin, s2cur);
                }
                int i;
                std::vector<double>::const_iterator bitp=benefits1.begin(),
                    bitend = benefits1.end();
                for (i=2; bitp!=bitend; ++bitp,++i) {
                    // compute accumulated similarity benefit of matching
                    // column c (first sequence) with multiple rows r
                    double rd_bft = *bitp + matrix(r-i,c-1).asb;
                    if (rd_bft > cur.asb) {
                        set(cur, rd_bft, 4, i);
                    }
                }
                std::vector<double> benefits2;
                if (c>1) {
                    benefits2 = m_ccal.match_range_with2(s1begin, s1cur, n2);
                }
                bitp=benefits2.begin();
                bitend = benefits2.end();
                for (i=2; bitp!=bitend; ++bitp,++i) {
                    // compute accumulated similarity benefit of matching
                    // multiple columns c (first sequence) with row r
                    double rd_bft = *bitp + matrix(r-1,c-i).asb;
                    if (rd_bft > cur.asb) {
                        set(cur, rd_bft, 5, i);
                    }
                }
            }
        }
    }

    template <class CostCalculator>
    template <class Sequence1>
    void ConsolidatingAligner<CostCalculator>::gap_init_first_column(
                Geert_cs_uu_nl::Matrix<detail::FrCell> &matrix,
                Sequence1 const &seq1) const
    {
        assert(boost::distance(seq1)+1 == matrix.row_size());
        typename boost::range_iterator<const Sequence1>::type cur_note=
            boost::const_begin(seq1);
        double cur_cost = 0.0;
        for (size_t i=1; i<matrix.row_size();++i) {
            cur_cost += m_ccal.match_gap_with2(*cur_note++);
            set(matrix(i,0), cur_cost, 2);
        }
    }

    template <class CostCalculator>
    template <class Sequence0>
    void ConsolidatingAligner<CostCalculator>::gap_init_first_row(
            Geert_cs_uu_nl::Matrix<detail::FrCell> &matrix,
                Sequence0 const &seq0) const
    {
        assert(boost::distance(seq0)+1 == matrix.column_size());
        typename boost::range_iterator<const Sequence0>::type cur_note=
            boost::const_begin(seq0);
        double cur_cost = 0.0;
        for (size_t i=1; i<matrix.column_size();++i) {
            cur_cost += m_ccal.match_gap_with1(*cur_note++);
            set(matrix(0,i), cur_cost, 1);
        }
    }

    template <class CostCalculator>
    template <class Sequence0, class Sequence1>
    void ConsolidatingAligner<CostCalculator>::gap_init(
                Geert_cs_uu_nl::Matrix<detail::FrCell> &matrix,
                Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        set(matrix(0, 0), 0.0, 3);
        gap_init_first_row(matrix,seq0);
        gap_init_first_column(matrix,seq1);
    }

    template <class CostCalculator>
    template <class Sequence0, class Sequence1>
    Alignment ConsolidatingAligner<CostCalculator>::
    global_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz0 = boost::distance(seq0);
	size_t sz1 = boost::distance(seq1);
        m::Matrix<FrCell> matrix(sz1+1, sz0+1, m::Uninit());
        gap_init(matrix, seq0, seq1);
        fill_matrix_std(matrix, boost::const_begin(seq0),
                boost::const_begin(seq1));
        // now trace back to find the best alignment
        return trace_back(matrix, sz1, sz0);
    }

    template <class CostCalculator>
    template <class Sequence0, class Sequence1>
    Alignment ConsolidatingAligner<CostCalculator>::
    cut_first_end_off_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz0 = boost::distance(seq0);
	size_t sz1 = boost::distance(seq1);
        m::Matrix<FrCell> matrix(sz1+1, sz0+1, m::Uninit());
        gap_init(matrix, seq0, seq1);
        fill_matrix_std(matrix, boost::const_begin(seq0),
                boost::const_begin(seq1));
        // now trace back to find the best alignment
        size_t best_c = matrix.column_size()-1;
        size_t best_r = matrix.row_size()-1;
        double best=matrix(best_r, best_c).asb;
        find_better_in_row(best, best_r, best_c, matrix);
        //find_better_in_column(best, best_r, best_c, matrix);
        return trace_back(matrix, best_r, best_c);
    }

    template <class CostCalculator>
    template <class Sequence0, class Sequence1>
    Alignment ConsolidatingAligner<CostCalculator>::
    cut_second_end_off_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz0 = boost::distance(seq0);
	size_t sz1 = boost::distance(seq1);
        m::Matrix<FrCell> matrix(sz1+1, sz0+1, m::Uninit());
        gap_init(matrix, seq0, seq1);
        fill_matrix_std(matrix, boost::const_begin(seq0),
                boost::const_begin(seq1));
        // now trace back to find the best alignment
        size_t best_c = matrix.column_size()-1;
        size_t best_r = matrix.row_size()-1;
        double best=matrix(best_r, best_c).asb;
        //find_better_in_row(best, best_r, best_c, matrix);
        find_better_in_column(best, best_r, best_c, matrix);
        return trace_back(matrix, best_r, best_c);
    }

    template <class CostCalculator>
    template <class Sequence0, class Sequence1>
    Alignment ConsolidatingAligner<CostCalculator>::
    cut_one_end_off_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz0 = boost::distance(seq0);
	size_t sz1 = boost::distance(seq1);
        m::Matrix<FrCell> matrix(sz1+1, sz0+1, m::Uninit());
        gap_init(matrix, seq0, seq1);
        fill_matrix_std(matrix, boost::const_begin(seq0),
                boost::const_begin(seq1));
        // now trace back to find the best alignment
        size_t best_c = matrix.column_size()-1;
        size_t best_r = matrix.row_size()-1;
        double best=matrix(best_r, best_c).asb;
        find_better_in_row(best, best_r, best_c, matrix);
        find_better_in_column(best, best_r, best_c, matrix);
        return trace_back(matrix, best_r, best_c);
    }

    template <class CostCalculator>
    template <class Sequence0, class Sequence1>
    Alignment ConsolidatingAligner<CostCalculator>::
    cut_one_begin_off_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz0 = boost::distance(seq0);
	size_t sz1 = boost::distance(seq1);
        m::Matrix<FrCell> matrix(sz1+1, sz0+1, m::Uninit());
        zero_init(matrix);
        fill_matrix_std(matrix, boost::const_begin(seq0),
                boost::const_begin(seq1));
        // now trace back to find the best alignment
        return trace_back(matrix, sz1, sz0);
    }

    template <class CostCalculator>
    template <class Sequence0, class Sequence1>
    Alignment ConsolidatingAligner<CostCalculator>::
    initial_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz0 = boost::distance(seq0);
	size_t sz1 = boost::distance(seq1);
        m::Matrix<FrCell> matrix(sz1+1, sz0+1, m::Uninit());
        gap_init(matrix, seq0, seq1);
        fill_matrix_std(matrix, boost::const_begin(seq0),
                boost::const_begin(seq1));
        size_t best_c;
        size_t best_r;
        double best;
        find_best(best, best_r, best_c, matrix);
        return trace_back(matrix, best_r, best_c);
    }

    template <class CostCalculator>
    template <class Sequence0, class Sequence1>
    Alignment ConsolidatingAligner<CostCalculator>::
    end_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz0 = boost::distance(seq0);
	size_t sz1 = boost::distance(seq1);
        m::Matrix<FrCell> matrix(sz1+1, sz0+1, m::Uninit());
        zero_init(matrix);
        fill_matrix_partial(matrix, boost::const_begin(seq0),
                boost::const_begin(seq1));
        // now trace back to find the best alignment
        return trace_back(matrix, sz1, sz0);
    }

    template <class CostCalculator>
    template <class Sequence0, class Sequence1>
    Alignment ConsolidatingAligner<CostCalculator>::
    semi_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz0 = boost::distance(seq0);
	size_t sz1 = boost::distance(seq1);
        m::Matrix<FrCell> matrix(sz1+1, sz0+1, m::Uninit());
        zero_init(matrix);
        fill_matrix_std(matrix, boost::const_begin(seq0),
                boost::const_begin(seq1));
        // now trace back to find the best alignment
        size_t best_c = matrix.column_size()-1;
        size_t best_r = matrix.row_size()-1;
        double best=matrix(best_r, best_c).asb;
        find_better_in_row(best, best_r, best_c, matrix);
        find_better_in_column(best, best_r, best_c, matrix);
        return trace_back(matrix, best_r, best_c);
    }

    template <class CostCalculator>
    template <class Sequence0, class Sequence1>
    Alignment ConsolidatingAligner<CostCalculator>::
    inside_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz0 = boost::distance(seq0);
	size_t sz1 = boost::distance(seq1);
        m::Matrix<FrCell> matrix(sz1+1, sz0+1, m::Uninit());
        set(matrix(0, 0), 0.0, 3);
        gap_init_first_row(matrix, seq0);
        zero_init_first_column(matrix);
        fill_matrix_std(matrix, boost::const_begin(seq0),
                boost::const_begin(seq1));
        // now trace back to find the best alignment
        size_t best_c = matrix.column_size()-1;
        size_t best_r = matrix.row_size()-1;
        double best=matrix(best_r, best_c).asb;
        find_better_in_column(best, best_r, best_c, matrix);
        return trace_back(matrix, best_r, best_c);
    }

    template <class CostCalculator>
    template <class Sequence0, class Sequence1>
    Alignment ConsolidatingAligner<CostCalculator>::
    partial_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz0 = boost::distance(seq0);
	size_t sz1 = boost::distance(seq1);
        m::Matrix<FrCell> matrix(sz1+1, sz0+1, m::Uninit());
        zero_init(matrix);
        fill_matrix_partial(matrix, boost::const_begin(seq0),
                boost::const_begin(seq1));
        // now trace back to find the best alignment
        size_t best_c;
        size_t best_r;
        double best;
        find_best(best, best_r, best_c, matrix);
        return trace_back(matrix, best_r, best_c);
    }


}
// vim:set shiftwidth=4 softtabstop=4 expandtab cindent:

#endif // MUSALIGNA7GDNETYQI385MDKWOWOOA
