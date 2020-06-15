#ifndef MUSALIGNA7GDNETYQI385MDKWOWIP
#define MUSALIGNA7GDNETYQI385MDKWOWIP

// Author: Geert-Jan Giezeman <G.J.Giezeman@uu.nl>
// Copyright: Utrecht University, 2012-2017

#include <cassert>
#include "boost/range.hpp"

namespace musaline {

    namespace detail {

        inline void set(NwCell &cell, double a, int fr)
        {
            assert(0<=fr && fr<4);
            cell.asb=a;
            cell.from=fr;
        }

        inline void set(NwgCell &result, double tl, double l, double t,
                int m, bool extended_gap_1, bool extended_gap_2)
        {
            assert(0<=m && m<4);
            result.asb[0] = tl;
            result.asb[1] = l;
            result.asb[2] = t;
            result.from = m;
            result.ext_1 = extended_gap_1;
            result.ext_2 = extended_gap_2;
        }

        template <class PPCostCalc>
        class CostCalculator1 {
            PPCostCalc *ori_calc; // not owned, *ori_calc must remain valid during use
            public:
            CostCalculator1() : ori_calc(0) {}
            CostCalculator1(PPCostCalc *o): ori_calc(o) {}
            template <class A, class B>
            void preprocess(A const & a, B const &b)
            {
                ori_calc->preprocess(a,b);
            }
            template <class A, class B>
            double match(A const & a, B const &b) const
            {
                return ori_calc->match(a,b);
            }
        };
    }

/*
 * LinearAligner (Needleman, Wunsch)
 * We compute the best alignment by filling in a matrix. 
 * The matrix holds one column for every element of sequence 1, plus one
 * (for border condition).
 * The matrix holds one row for every element of sequence 2, plus one
 * (for border condition).
 * The matrix element (r+1,c+1) holds the best way of matching sequence 1 up
 * to and including c with sequence 2 up to and including r.
 * We maintain a score value asb, for accumulated similarity benefit
 * We also record a value 'from', where we record what the previous position was
 * - from==0 : align elements c and r of the sequences 1 and 2
 * - from==1 : align element c of sequence 1 with a gap
 * - from==2 : align element r of sequence 2 with a gap
 * - from==3 : start alignment here
 */

    namespace detail {

    void gap_init_first_column(Matrix<NwCell> &matrix, double gap_bft)
    {
        double cur_cost = gap_bft;
        Matrix<NwCell>::CursorType crs(matrix.get_cursor(1,0));
        for (;crs.row()<matrix.row_size();crs.inc_row()) {
            set(matrix(crs), cur_cost, 2);
            cur_cost += gap_bft;
        }
    }
    void gap_init_first_row(Matrix<NwCell> &matrix, double gap_bft)
    {
        double cur_cost = gap_bft;
        Matrix<NwCell>::CursorType crs(matrix.get_cursor(0,1));
        for (; crs.col()<matrix.column_size();crs.inc_col()) {
            set(matrix(crs), cur_cost, 1);
            cur_cost += gap_bft;
        }
    }
    void gap_init(Matrix<NwCell> &matrix, double gap_bft)
    {
        set(matrix(0, 0), 0.0, 3);
        gap_init_first_row(matrix,gap_bft);
        gap_init_first_column(matrix,gap_bft);
    }

    void zero_init_first_column(Matrix<NwCell> &matrix)
    {
        Matrix<NwCell>::CursorType crs(matrix.get_cursor(1,0));
        for (;crs.row()<matrix.row_size();crs.inc_row()) {
            set(matrix(crs), 0.0, 3);
        }
    }
    void zero_init_first_row(Matrix<NwCell> &matrix)
    {
        Matrix<NwCell>::CursorType crs(matrix.get_cursor(0,1));
        for (; crs.col()<matrix.column_size();crs.inc_col()) {
            set(matrix(crs), 0.0, 3);
        }
    }
    void zero_init(Matrix<NwCell> &matrix)
    {
        set(matrix(0, 0), 0.0, 3);
        zero_init_first_row(matrix);
        zero_init_first_column(matrix);
    }

    void find_better_in_column(double &best, size_t&cur_r, size_t&cur_c, 
            Matrix<NwCell> const &matrix)
    {
        size_t c = matrix.column_size()-1;
        for (size_t r=0; r<matrix.row_size();++r) {
            NwCell const&cell = matrix(r,c);
            if (cell.asb>best) {
                best=cell.asb;
                cur_r=r;
                cur_c=c;
            }
        }
    }

    void find_better_in_row(double &best, size_t&cur_r, size_t&cur_c, 
            Matrix<NwCell> const &matrix)
    {
        size_t r = matrix.row_size()-1;
        for (size_t c=0; c<matrix.column_size();++c) {
            NwCell const&cell = matrix(r,c);
            if (cell.asb>best) {
                best=cell.asb;
                cur_r=r;
                cur_c=c;
            }
        }
    }
    void find_best(double &best, size_t &cur_r, size_t &cur_c, 
            Matrix<NwCell> const &matrix)
    {
        best = 0.0;
        cur_r=0;
        cur_c=0;
        for (size_t r=1; r<matrix.row_size();++r) {
            for (size_t c=1; c<matrix.column_size();++c) {
                NwCell const&cell = matrix(r,c);
                if (cell.asb>best) {
                    best=cell.asb;
                    cur_r=r;
                    cur_c=c;
                }
            }
        }
    }

    Alignment trace_back(Matrix<NwCell> &matrix,
        size_t cur_r, size_t cur_c)
    {
        Alignment result;
        result.score = matrix(cur_r, cur_c).asb;
        result.end[0]=cur_c;
        result.end[1]=cur_r;
        std::vector<Match> al;
        al.reserve(cur_c+cur_r);
        bool tracking=true;
        NwCell const *cur = &matrix(cur_r, cur_c);
        double to_score=result.score;
        while (tracking) {
            Match match;
            // check which way to go
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
            default:
                assert(false);
                throw std::logic_error("");
            }
            if (!tracking)
                break;
            cur = &matrix(cur_r, cur_c);
            double from_score=cur->asb;
            match.score= to_score-from_score;
            al.push_back(match);
            to_score= from_score;
        }
        result.start[0]=cur_c;
        result.start[1]=cur_r;
        std::reverse_copy(al.begin(), al.end(),
                std::back_inserter(result.matches));
        return result;
    }

    void fill_cell(NwCell &cur, NwCell const &tl,
            NwCell const &l, NwCell const &t,
            double match_bft, double gap_bft)
    {
        // compute accumulated similarity benefit of matching
        // column c with row r
        double rd_bft = match_bft + tl.asb;
        if (rd_bft > cur.asb) {
            cur.asb = rd_bft;
            cur.from = 0;
        }
        // compute accumulated similarity benefit of matching column c with gap
        double right_bft = gap_bft + l.asb;
        if (right_bft > cur.asb) {
            cur.asb = right_bft;
            cur.from = 1;
        }
        // compute accumulated similarity benefit of matching row r with gap
        double down_bft = gap_bft + t.asb;
        if (down_bft > cur.asb) {
            cur.asb = down_bft;
            cur.from = 2;
        }
    }
    }

    template <class CostCalculator>
    template <class Iter1, class Iter2>
    void LinearAligner<CostCalculator>::fill_matrix_std(
            Geert_cs_uu_nl::Matrix<detail::NwCell> &matrix,
            Iter1 s1begin, Iter2 s2begin) const
    {
        using namespace detail;;
        typedef Geert_cs_uu_nl::Matrix<NwCell> Mat;
        Mat::CursorType crs0(matrix.get_cursor(0,0));
        NwCell *tl, *t, *l, *cur;

        Iter2 s2cur(s2begin);
        for (size_t r=1; r<matrix.row_size(); ++r, ++s2cur) {
            Mat::CursorType crs1(crs0);
            crs0.inc_row();
            Mat::CursorType crs2(crs0);
            t=&matrix(crs1);
            cur=&matrix(crs2);
            Iter1 s1cur(s1begin);

            for (size_t c=1; c<matrix.column_size(); ++c, ++s1cur) {
                tl = t;
                l = cur;
                crs1.inc_col();
                t=&matrix(crs1);
                crs2.inc_col();
                cur=&matrix(crs2);
                cur->asb = mininf;
                fill_cell(*cur, *tl, *l, *t,
                        m_ccal.match(*s1cur, *s2cur),
                        m_ccal.gap_bft());
            }
        }
    }

    template <class CostCalculator>
    template <class Iter1, class Iter2>
    void LinearAligner<CostCalculator>::fill_matrix_partial(
            Geert_cs_uu_nl::Matrix<detail::NwCell> &matrix,
            Iter1 s1cur, Iter2 s2begin) const
    {
        using namespace detail;
        for (size_t c=1; c<matrix.column_size(); ++c, ++s1cur) {
            Iter2 s2cur(s2begin);
            for (size_t r=1; r<matrix.row_size(); ++r, ++s2cur) {
                NwCell &cur = matrix(r, c);
                cur.asb = 0.0;
                cur.from = 3;
                fill_cell(cur, matrix(r-1, c-1), matrix(r, c-1),
                        matrix(r-1, c),m_ccal.match(*s1cur, *s2cur),
                        m_ccal.gap_bft());
            }
        }
    }

    template <class CostCalculator>
    template <class Sequence0, class Sequence1>
    Alignment LinearAligner<CostCalculator>::
    global_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz1 = boost::distance(seq0);
	size_t sz2 = boost::distance(seq1);
        m::Matrix<NwCell> matrix(sz2+1, sz1+1, m::Uninit());
        gap_init(matrix, m_ccal.gap_bft());
        fill_matrix_std(matrix, boost::const_begin(seq0),
                boost::const_begin(seq1));
        // now trace back to find the best alignment
        return trace_back(matrix, sz2, sz1);
    }

    template <class CostCalculator>
    template <class Sequence0, class Sequence1>
    Alignment LinearAligner<CostCalculator>::
    cut_one_end_off_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz1 = boost::distance(seq0);
	size_t sz2 = boost::distance(seq1);
        m::Matrix<NwCell> matrix(sz2+1, sz1+1, m::Uninit());
        gap_init(matrix, m_ccal.gap_bft());
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
    Alignment LinearAligner<CostCalculator>::
    cut_first_end_off_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz1 = boost::distance(seq0);
	size_t sz2 = boost::distance(seq1);
        m::Matrix<NwCell> matrix(sz2+1, sz1+1, m::Uninit());
        gap_init(matrix, m_ccal.gap_bft());
        fill_matrix_std(matrix, boost::const_begin(seq0),
                boost::const_begin(seq1));
        // now trace back to find the best alignment
        size_t best_c = matrix.column_size()-1;
        size_t best_r = matrix.row_size()-1;
        double best=matrix(best_r, best_c).asb;
        find_better_in_row(best, best_r, best_c, matrix);
        // find_better_in_column(best, best_r, best_c, matrix);
        return trace_back(matrix, best_r, best_c);
    }

    template <class CostCalculator>
    template <class Sequence0, class Sequence1>
    Alignment LinearAligner<CostCalculator>::
    cut_second_end_off_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz1 = boost::distance(seq0);
	size_t sz2 = boost::distance(seq1);
        m::Matrix<NwCell> matrix(sz2+1, sz1+1, m::Uninit());
        gap_init(matrix, m_ccal.gap_bft());
        fill_matrix_std(matrix, boost::const_begin(seq0),
                boost::const_begin(seq1));
        // now trace back to find the best alignment
        size_t best_c = matrix.column_size()-1;
        size_t best_r = matrix.row_size()-1;
        double best=matrix(best_r, best_c).asb;
        // find_better_in_row(best, best_r, best_c, matrix);
        find_better_in_column(best, best_r, best_c, matrix);
        return trace_back(matrix, best_r, best_c);
    }

    template <class CostCalculator>
    template <class Sequence0, class Sequence1>
    Alignment LinearAligner<CostCalculator>::
    cut_one_begin_off_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz1 = boost::distance(seq0);
	size_t sz2 = boost::distance(seq1);
        m::Matrix<NwCell> matrix(sz2+1, sz1+1, m::Uninit());
        zero_init(matrix);
        fill_matrix_std(matrix, boost::const_begin(seq0),
                boost::const_begin(seq1));
        // now trace back to find the best alignment
        return trace_back(matrix, sz2, sz1);
    }

    template <class CostCalculator>
    template <class Sequence0, class Sequence1>
    Alignment LinearAligner<CostCalculator>::
    initial_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz1 = boost::distance(seq0);
	size_t sz2 = boost::distance(seq1);
        m::Matrix<NwCell> matrix(sz2+1, sz1+1, m::Uninit());
        gap_init(matrix, m_ccal.gap_bft());
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
    Alignment LinearAligner<CostCalculator>::
    end_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz1 = boost::distance(seq0);
	size_t sz2 = boost::distance(seq1);
        m::Matrix<NwCell> matrix(sz2+1, sz1+1, m::Uninit());
        zero_init(matrix);
        fill_matrix_partial(matrix, boost::const_begin(seq0),
                boost::const_begin(seq1));
        // now trace back to find the best alignment
        return trace_back(matrix, sz2, sz1);
    }

    template <class CostCalculator>
    template <class Sequence0, class Sequence1>
    Alignment LinearAligner<CostCalculator>::
    semi_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz1 = boost::distance(seq0);
	size_t sz2 = boost::distance(seq1);
        m::Matrix<NwCell> matrix(sz2+1, sz1+1, m::Uninit());
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
    Alignment LinearAligner<CostCalculator>::
    inside_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz1 = boost::distance(seq0);
	size_t sz2 = boost::distance(seq1);
        m::Matrix<NwCell> matrix(sz2+1, sz1+1, m::Uninit());
        set(matrix(0, 0), 0.0, 3);
        gap_init_first_row(matrix, m_ccal.gap_bft());
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
    Alignment LinearAligner<CostCalculator>::
    partial_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz1 = boost::distance(seq0);
	size_t sz2 = boost::distance(seq1);
        m::Matrix<NwCell> matrix(sz2+1, sz1+1, m::Uninit());
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

/*
 * LinearAligner2
 */

    template <class CostCalculator>
    template <class Sequence1>
    void LinearAligner2<CostCalculator>::gap_init_first_column(
                Geert_cs_uu_nl::Matrix<detail::NwCell> &matrix,
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
    void LinearAligner2<CostCalculator>::gap_init_first_row(
            Geert_cs_uu_nl::Matrix<detail::NwCell> &matrix,
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
    void LinearAligner2<CostCalculator>::gap_init(
                Geert_cs_uu_nl::Matrix<detail::NwCell> &matrix,
                Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        set(matrix(0, 0), 0.0, 3);
        gap_init_first_row(matrix,seq0);
        gap_init_first_column(matrix,seq1);
    }

    template <class CostCalculator>
    template <class Iter1, class Iter2>
    void LinearAligner2<CostCalculator>::fill_matrix_std(
            Geert_cs_uu_nl::Matrix<detail::NwCell> &matrix,
            Iter1 s1begin, Iter2 s2begin) const
    {
        using namespace detail;;
        typedef Geert_cs_uu_nl::Matrix<NwCell> Mat;
        Mat::CursorType crs0(matrix.get_cursor(0,0));
        NwCell *tl, *t, *l, *cur;

        Iter2 s2cur(s2begin);
        for (size_t r=1; r<matrix.row_size(); ++r, ++s2cur) {
            Mat::CursorType crs1(crs0);
            crs0.inc_row();
            Mat::CursorType crs2(crs0);
            t=&matrix(crs1);
            cur=&matrix(crs2);
            Iter1 s1cur(s1begin);

            for (size_t c=1; c<matrix.column_size(); ++c, ++s1cur) {
                tl = t;
                l = cur;
                crs1.inc_col();
                t=&matrix(crs1);
                crs2.inc_col();
                cur=&matrix(crs2);
                cur->asb = mininf;
                fill_cell(*cur, *tl, *l, *t,
                        m_ccal.match(*s1cur, *s2cur),
                        m_ccal.gap_bft());
            }
        }
    }

    template <class CostCalculator>
    template <class Iter1, class Iter2>
    void LinearAligner2<CostCalculator>::fill_matrix_partial(
            Geert_cs_uu_nl::Matrix<detail::NwCell> &matrix,
            Iter1 s1cur, Iter2 s2begin) const
    {
        using namespace detail;
        for (size_t c=1; c<matrix.column_size(); ++c, ++s1cur) {
            Iter2 s2cur(s2begin);
            for (size_t r=1; r<matrix.row_size(); ++r, ++s2cur) {
                NwCell &cur = matrix(r, c);
                cur.asb = 0.0;
                cur.from = 3;
                fill_cell(cur, matrix(r-1, c-1), matrix(r, c-1),
                        matrix(r-1, c),m_ccal.match(*s1cur, *s2cur),
                        m_ccal.gap_bft());
            }
        }
    }

    template <class CostCalculator>
    template <class Sequence0, class Sequence1>
    Alignment LinearAligner2<CostCalculator>::
    global_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz1 = boost::distance(seq0);
	size_t sz2 = boost::distance(seq1);
        m::Matrix<NwCell> matrix(sz2+1, sz1+1, m::Uninit());
        gap_init(matrix, seq0, seq1);
        fill_matrix_std(matrix, boost::const_begin(seq0),
                boost::const_begin(seq1));
        // now trace back to find the best alignment
        return trace_back(matrix, sz2, sz1);
    }

    template <class CostCalculator>
    template <class Sequence0, class Sequence1>
    Alignment LinearAligner2<CostCalculator>::
    cut_one_end_off_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz1 = boost::distance(seq0);
	size_t sz2 = boost::distance(seq1);
        m::Matrix<NwCell> matrix(sz2+1, sz1+1, m::Uninit());
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
    Alignment LinearAligner2<CostCalculator>::
    cut_first_end_off_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz1 = boost::distance(seq0);
	size_t sz2 = boost::distance(seq1);
        m::Matrix<NwCell> matrix(sz2+1, sz1+1, m::Uninit());
        gap_init(matrix, seq0, seq1);
        fill_matrix_std(matrix, boost::const_begin(seq0),
                boost::const_begin(seq1));
        // now trace back to find the best alignment
        size_t best_c = matrix.column_size()-1;
        size_t best_r = matrix.row_size()-1;
        double best=matrix(best_r, best_c).asb;
        find_better_in_row(best, best_r, best_c, matrix);
        // find_better_in_column(best, best_r, best_c, matrix);
        return trace_back(matrix, best_r, best_c);
    }

    template <class CostCalculator>
    template <class Sequence0, class Sequence1>
    Alignment LinearAligner2<CostCalculator>::
    cut_second_end_off_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz1 = boost::distance(seq0);
	size_t sz2 = boost::distance(seq1);
        m::Matrix<NwCell> matrix(sz2+1, sz1+1, m::Uninit());
        gap_init(matrix, seq0, seq1);
        fill_matrix_std(matrix, boost::const_begin(seq0),
                boost::const_begin(seq1));
        // now trace back to find the best alignment
        size_t best_c = matrix.column_size()-1;
        size_t best_r = matrix.row_size()-1;
        double best=matrix(best_r, best_c).asb;
        // find_better_in_row(best, best_r, best_c, matrix);
        find_better_in_column(best, best_r, best_c, matrix);
        return trace_back(matrix, best_r, best_c);
    }

    template <class CostCalculator>
    template <class Sequence0, class Sequence1>
    Alignment LinearAligner2<CostCalculator>::
    cut_one_begin_off_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz1 = boost::distance(seq0);
	size_t sz2 = boost::distance(seq1);
        m::Matrix<NwCell> matrix(sz2+1, sz1+1, m::Uninit());
        zero_init(matrix);
        fill_matrix_std(matrix, boost::const_begin(seq0),
                boost::const_begin(seq1));
        // now trace back to find the best alignment
        return trace_back(matrix, sz2, sz1);
    }

    template <class CostCalculator>
    template <class Sequence0, class Sequence1>
    Alignment LinearAligner2<CostCalculator>::
    initial_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz1 = boost::distance(seq0);
	size_t sz2 = boost::distance(seq1);
        m::Matrix<NwCell> matrix(sz2+1, sz1+1, m::Uninit());
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
    Alignment LinearAligner2<CostCalculator>::
    end_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz1 = boost::distance(seq0);
	size_t sz2 = boost::distance(seq1);
        m::Matrix<NwCell> matrix(sz2+1, sz1+1, m::Uninit());
        zero_init(matrix);
        fill_matrix_partial(matrix, boost::const_begin(seq0),
                boost::const_begin(seq1));
        // now trace back to find the best alignment
        return trace_back(matrix, sz2, sz1);
    }

    template <class CostCalculator>
    template <class Sequence0, class Sequence1>
    Alignment LinearAligner2<CostCalculator>::
    semi_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz1 = boost::distance(seq0);
	size_t sz2 = boost::distance(seq1);
        m::Matrix<NwCell> matrix(sz2+1, sz1+1, m::Uninit());
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
    Alignment LinearAligner2<CostCalculator>::
    inside_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz1 = boost::distance(seq0);
	size_t sz2 = boost::distance(seq1);
        m::Matrix<NwCell> matrix(sz2+1, sz1+1, m::Uninit());
        set(matrix(0, 0), 0.0, 3);
        gap_init(matrix, seq0, seq1);
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
    Alignment LinearAligner2<CostCalculator>::
    partial_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz1 = boost::distance(seq0);
	size_t sz2 = boost::distance(seq1);
        m::Matrix<NwCell> matrix(sz2+1, sz1+1, m::Uninit());
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

/*
 * AffineAligner (Needleman, Wunsch, Gotoh)
 * We compute the best alignment by filling in a matrix. 
 * The matrix holds one column for every element of sequence 1, plus one
 * (for border condition).
 * The matrix holds one row for every element of sequence 2, plus one
 * (for border condition).
 * The matrix element (r+1,c+1) holds the best way of matching sequence 1 up
 * to and including c with sequence 2 up to and including r.
 * We maintain three score values (asb, for accumulated similarity benefit),
 * where we record what the step was for reaching this cell:
 * - asb[0] : align elements c and r of the sequences 1 and 2
 * - asb[1] : align element c of sequence 1 with a gap
 * - asb[2] : align element r of sequence 2 with a gap
 * We need to maintain the three score values, because extending a gap is
 * possibly cheaper than starting a gap.
 * ext_1 and ext_2 hold a boolean that indicate whether asb[1] and asb[2] are
 * reached by extending a gap.
 * We also record a value 'from', where we record what the previous position was
 * - from==0 : align elements c and r of the sequences 1 and 2
 * - from==1 : align element c of sequence 1 with a gap
 * - from==2 : align element r of sequence 2 with a gap
 * - from==3 : start alignment here
 * If 'from' is not equal to 3, then
 * asb[from] == max(asb[0], asb[1], asb[2])
 */

    namespace detail {

    using Geert_cs_uu_nl::Matrix;

    inline double max_bft(NwgCell const &cell)
    {
        return cell.from==3 ? 0.0 : cell.asb[cell.from];
    }

    inline NwgCell make_nwg_cell(double tl, double l, double t,
        int m, bool extended_gap_1, bool extended_gap_2)
    {
        assert(0<=m && m<4);
        NwgCell result;
        result.asb[0] = tl;
        result.asb[1] = l;
        result.asb[2] = t;
        result.from = m;
        result.ext_1 = extended_gap_1;
        result.ext_2 = extended_gap_2;
        return result;
    }

    void gap_init_first_column(Matrix<NwgCell> &matrix,
            double gap_start, double gap_ext)
    {
        double cur_cost = gap_start;
        Matrix<NwCell>::CursorType crs(matrix.get_cursor(1,0));
        set(matrix(crs), mininf, mininf, cur_cost, 2, false, false);
        crs.inc_row();
        for (;crs.row()<matrix.row_size();crs.inc_row()) {
            cur_cost += gap_ext;
            set(matrix(crs), mininf, mininf, cur_cost, 2, false, true);
        }
    }

    void gap_init_first_row(Matrix<NwgCell> &matrix,
            double gap_start, double gap_ext)
    {
        double cur_cost = gap_start;
        set(matrix(0,1), mininf, cur_cost, mininf, 1, false, false);
        for (size_t i=2; i<matrix.column_size();++i) {
            cur_cost += gap_ext;
            set(matrix(0,i), mininf, cur_cost, mininf, 1, true, false);
        }
    }

    void gap_init(Matrix<NwgCell> &matrix, double gap_start, double gap_ext)
    {
        set(matrix(0,0), 0.0, 0.0, 0.0, 3, false, false);
        gap_init_first_row(matrix,gap_start, gap_ext);
        gap_init_first_column(matrix,gap_start, gap_ext);
    }


    void zero_init_first_column(Matrix<NwgCell> &matrix)
    {
        Matrix<NwCell>::CursorType crs(matrix.get_cursor(1,0));
        for (;crs.row()<matrix.row_size();crs.inc_row()) {
            set(matrix(crs), 0.0, mininf, mininf, 3, false, false);
        }
    }
    void zero_init_first_row(Matrix<NwgCell> &matrix)
    {
        for (size_t i=1; i<matrix.column_size();++i) {
            set(matrix(0,i), 0.0, mininf, mininf, 3, false, false);
        }
    }
    void zero_init(Matrix<NwgCell> &matrix)
    {
        set(matrix(0,0), 0.0, mininf, mininf, 3, false, false);
        zero_init_first_row(matrix);
        zero_init_first_column(matrix);
    }

    void find_better_in_column(double &best, size_t&cur_r, size_t&cur_c, 
            Matrix<NwgCell> const &matrix)
    {
        size_t c = matrix.column_size()-1;
        for (size_t r=0; r<matrix.row_size();++r) {
            NwgCell const&cell = matrix(r,c);
            double bft=max_bft(cell);
            if (bft>best) {
                best=bft;
                cur_r=r;
                cur_c=c;
            }
        }
    }

    void find_better_in_row(double &best, size_t&cur_r, size_t&cur_c, 
            Matrix<NwgCell> const &matrix)
    {
        size_t r = matrix.row_size()-1;
        for (size_t c=0; c<matrix.column_size();++c) {
            NwgCell const&cell = matrix(r,c);
            double bft=max_bft(cell);
            if (bft>best) {
                best=bft;
                cur_r=r;
                cur_c=c;
            }
        }
    }
    void find_best(double &best, size_t &cur_r, size_t &cur_c, 
            Matrix<NwgCell> const &matrix)
    {
        best = 0.0;
        cur_r=0;
        cur_c=0;
        for (size_t r=1; r<matrix.row_size();++r) {
            for (size_t c=1; c<matrix.column_size();++c) {
                NwgCell const&cell = matrix(r,c);
                double bft=max_bft(cell);
                if (bft>best) {
                    best=bft;
                    cur_r=r;
                    cur_c=c;
                }
            }
        }
    }

    void fill_cell( NwgCell &cur,
            NwgCell const &tl, NwgCell const &l, NwgCell const &t,
            double match_bft, double gap_start, double gap_ext, double cur_bft)
    {
        cur.asb[0] = match_bft + max_bft(tl);
        if (cur.asb[0]>cur_bft) {
            cur.from = 0;
            cur_bft = cur.asb[0];
        }
        // compute accumulated similarity benefit of matching column c with gap
        // First compute cost of extending the gap.
        cur.asb[1] = gap_ext + l.asb[1];
        cur.ext_1 = true;
        if (l.from != 1) {
            // cost of starting a new gap
            double new_gap_cost = gap_start + max_bft(l);
            if (new_gap_cost > cur.asb[1]) {
                cur.asb[1] = new_gap_cost;
                cur.ext_1 = false;
            }
        }
        if (cur.asb[1] > cur_bft) {
            cur_bft = cur.asb[1];
            cur.from = 1;
        }
        // compute accumulated similarity benefit of matching row r with gap
        // First compute cost of extending the gap.
        cur.asb[2] = gap_ext + t.asb[2];
        cur.ext_2 = true;
        if (t.from != 2) {
            // cost of starting a new gap
            double new_gap_cost = gap_start + max_bft(t);
            if (new_gap_cost > cur.asb[2]) {
                cur.asb[2] = new_gap_cost;
                cur.ext_2 = false;
            }
        }
        if (cur.asb[2] > cur_bft) {
            cur_bft = cur.asb[2];
            cur.from = 2;
        }
    }

    Alignment trace_back(Matrix<NwgCell> &matrix,
        size_t cur_r, size_t cur_c)
    {
        Alignment result;
        result.score = max_bft(matrix(cur_r, cur_c));
        result.end[0]=cur_c;
        result.end[1]=cur_r;
        std::vector<Match> al;
        al.reserve(cur_c+cur_r);
        bool tracking=true;
        NwgCell const *cur = &matrix(cur_r, cur_c);
        int dir = cur->from;
        double to_score=result.score;
        while (true) {
            Match match;
            bool need_extension=false;
            // check which way to go
            switch (dir) {
            case 0:
                match.n[0]=1;
                match.n[1]=1;
                --cur_c;
                --cur_r;
                // need_extension = false;
                break;
            case 1:
                match.n[0]=1;
                match.n[1]=0;
                need_extension = cur->ext_1;
                --cur_c;
                break;
            case 2:
                match.n[0]=0;
                match.n[1]=1;
                need_extension = cur->ext_2;
                --cur_r;
                break;
            case 3:
                tracking=false;
                break;
            default:
                assert(false);
                throw std::logic_error("");
            }
            if (!tracking)
                break;
            cur= &matrix(cur_r, cur_c);
            double from_score=need_extension?cur->asb[dir] : max_bft(*cur);
            match.score= to_score-from_score;
            to_score=from_score;
            al.push_back(match);
            if (!need_extension)
                dir = cur->from;
        }
        result.start[0]=cur_c;
        result.start[1]=cur_r;
        std::reverse_copy(al.begin(), al.end(),
                std::back_inserter(result.matches));
        return result;
    }
    }

    template <class CostCalculator>
    template <class Iter1, class Iter2>
    void AffineAligner<CostCalculator>::fill_matrix_std(
            Geert_cs_uu_nl::Matrix<detail::NwgCell> &matrix,
            Iter1 s1cur, Iter2 s2begin) const
    {
        using namespace detail;
        typedef Geert_cs_uu_nl::Matrix<NwgCell> Mat;
        Mat::CursorType crs0(matrix.get_cursor(0,0));
        NwgCell *tl, *t, *l, *cur;

        for (; crs0.col()+1<matrix.column_size(); ++s1cur) {
            l=&matrix(crs0);
            Mat::CursorType crs1(crs0);
            crs0.inc_col();
            cur=&matrix(crs0);
            Mat::CursorType crs2(crs0);
            Iter2 s2cur(s2begin);
            for (size_t r=1; r<matrix.row_size(); ++r, ++s2cur) {
                tl = l;
                t = cur;
                crs1.inc_row();
                l=&matrix(crs1);
                crs2.inc_row();
                cur=&matrix(crs2);
                //cur->asb = mininf;
                fill_cell(*cur, *tl, *l, *t,
                        m_ccal.match(*s1cur, *s2cur),
                        m_ccal.gap_bft(), m_ccal.ext_bft(), mininf);
            }
        }
    }

    template <class CostCalculator>
    template <class Iter1, class Iter2>
    void AffineAligner<CostCalculator>::fill_matrix_partial(
            Geert_cs_uu_nl::Matrix<detail::NwgCell> &matrix,
            Iter1 s1cur, Iter2 s2begin) const
    {
        using namespace detail;
        for (size_t c=1; c<matrix.column_size(); ++c, ++s1cur) {
            Iter2 s2cur(s2begin);
            for (size_t r=1; r<matrix.row_size(); ++r, ++s2cur) {
                NwgCell &cur = matrix(r, c);
                cur.from=3;
                fill_cell(cur, matrix(r-1, c-1), matrix(r, c-1),
                        matrix(r-1, c), m_ccal.match(*s1cur, *s2cur),
                        m_ccal.gap_bft(), m_ccal.ext_bft(), 0.0);
            }
        }
    }

    template <class CostCalculator>
    template <class Sequence0, class Sequence1>
    Alignment AffineAligner<CostCalculator>::
    global_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz1 = boost::distance(seq0);
	size_t sz2 = boost::distance(seq1);
        m::Matrix<NwgCell> matrix(sz2+1, sz1+1, m::Uninit());
        gap_init(matrix, m_ccal.gap_bft(), m_ccal.ext_bft());
        fill_matrix_std(matrix, boost::const_begin(seq0),
                boost::const_begin(seq1));
        return trace_back(matrix, sz2, sz1);
    }

    template <class CostCalculator>
    template <class Sequence0, class Sequence1>
    Alignment AffineAligner<CostCalculator>::
    cut_one_end_off_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz1 = boost::distance(seq0);
	size_t sz2 = boost::distance(seq1);
        m::Matrix<NwgCell> matrix(sz2+1, sz1+1, m::Uninit());
        gap_init(matrix, m_ccal.gap_bft(), m_ccal.ext_bft());
        fill_matrix_std(matrix, boost::const_begin(seq0),
                boost::const_begin(seq1));
        // now trace back to find the best alignment
        size_t best_c = matrix.column_size()-1;
        size_t best_r = matrix.row_size()-1;
        double best=max_bft(matrix(best_r, best_c));
        find_better_in_row(best, best_r, best_c, matrix);
        find_better_in_column(best, best_r, best_c, matrix);
        return trace_back(matrix, best_r, best_c);
    }

    template <class CostCalculator>
    template <class Sequence0, class Sequence1>
    Alignment AffineAligner<CostCalculator>::
    cut_first_end_off_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz1 = boost::distance(seq0);
	size_t sz2 = boost::distance(seq1);
        m::Matrix<NwgCell> matrix(sz2+1, sz1+1, m::Uninit());
        gap_init(matrix, m_ccal.gap_bft(), m_ccal.ext_bft());
        fill_matrix_std(matrix, boost::const_begin(seq0),
                boost::const_begin(seq1));
        // now trace back to find the best alignment
        size_t best_c = matrix.column_size()-1;
        size_t best_r = matrix.row_size()-1;
        double best=max_bft(matrix(best_r, best_c));
        find_better_in_row(best, best_r, best_c, matrix);
        // find_better_in_column(best, best_r, best_c, matrix);
        return trace_back(matrix, best_r, best_c);
    }

    template <class CostCalculator>
    template <class Sequence0, class Sequence1>
    Alignment AffineAligner<CostCalculator>::
    cut_second_end_off_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz1 = boost::distance(seq0);
	size_t sz2 = boost::distance(seq1);
        m::Matrix<NwgCell> matrix(sz2+1, sz1+1, m::Uninit());
        gap_init(matrix, m_ccal.gap_bft(), m_ccal.ext_bft());
        fill_matrix_std(matrix, boost::const_begin(seq0),
                boost::const_begin(seq1));
        // now trace back to find the best alignment
        size_t best_c = matrix.column_size()-1;
        size_t best_r = matrix.row_size()-1;
        double best=max_bft(matrix(best_r, best_c));
        // find_better_in_row(best, best_r, best_c, matrix);
        find_better_in_column(best, best_r, best_c, matrix);
        return trace_back(matrix, best_r, best_c);
    }

    template <class CostCalculator>
    template <class Sequence0, class Sequence1>
    Alignment AffineAligner<CostCalculator>::
    cut_one_begin_off_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz1 = boost::distance(seq0);
	size_t sz2 = boost::distance(seq1);
        m::Matrix<NwgCell> matrix(sz2+1, sz1+1, m::Uninit());
        zero_init(matrix);
        fill_matrix_std(matrix, boost::const_begin(seq0),
                boost::const_begin(seq1));
        // now trace back to find the best alignment
        return trace_back(matrix, sz2, sz1);
    }
    template <class CostCalculator>
    template <class Sequence0, class Sequence1>
    Alignment AffineAligner<CostCalculator>::
    initial_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz1 = boost::distance(seq0);
	size_t sz2 = boost::distance(seq1);
        m::Matrix<NwgCell> matrix(sz2+1, sz1+1, m::Uninit());
        gap_init(matrix, m_ccal.gap_bft(), m_ccal.ext_bft());
        fill_matrix_std(matrix, boost::const_begin(seq0),
                boost::const_begin(seq1));
        // now trace back to find the best alignment
        size_t best_c;
        size_t best_r;
        double best;
        find_best(best, best_r, best_c, matrix);
        return trace_back(matrix, best_r, best_c);
    }

    template <class CostCalculator>
    template <class Sequence0, class Sequence1>
    Alignment AffineAligner<CostCalculator>::
    end_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz1 = boost::distance(seq0);
	size_t sz2 = boost::distance(seq1);
        m::Matrix<NwgCell> matrix(sz2+1, sz1+1, m::Uninit());
        zero_init(matrix);
        fill_matrix_partial(matrix, boost::const_begin(seq0),
                boost::const_begin(seq1));
        // now trace back to find the best alignment
        return trace_back(matrix, sz2, sz1);
    }

    template <class CostCalculator>
    template <class Sequence0, class Sequence1>
    Alignment AffineAligner<CostCalculator>::
    semi_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz1 = boost::distance(seq0);
	size_t sz2 = boost::distance(seq1);
        m::Matrix<NwgCell> matrix(sz2+1, sz1+1, m::Uninit());
        zero_init(matrix);
        fill_matrix_std(matrix, boost::const_begin(seq0),
                boost::const_begin(seq1));
        // now trace back to find the best alignment
        size_t best_c = matrix.column_size()-1;
        size_t best_r = matrix.row_size()-1;
        double best=max_bft(matrix(best_r, best_c));
        find_better_in_row(best, best_r, best_c, matrix);
        find_better_in_column(best, best_r, best_c, matrix);
        return trace_back(matrix, best_r, best_c);
    }

    template <class CostCalculator>
    template <class Sequence0, class Sequence1>
    Alignment AffineAligner<CostCalculator>::
    inside_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz1 = boost::distance(seq0);
	size_t sz2 = boost::distance(seq1);
        m::Matrix<NwgCell> matrix(sz2+1, sz1+1, m::Uninit());
        set(matrix(0,0),0.0,0.0,0.0,3,false,false);
        gap_init_first_row(matrix, m_ccal.gap_bft(), m_ccal.ext_bft());
        zero_init_first_column(matrix);
        fill_matrix_std(matrix, boost::const_begin(seq0),
                boost::const_begin(seq1));
        // now trace back to find the best alignment
        size_t best_c = matrix.column_size()-1;
        size_t best_r = matrix.row_size()-1;
        double best=max_bft(matrix(best_r, best_c));
        find_better_in_column(best, best_r, best_c, matrix);
        return trace_back(matrix, best_r, best_c);
    }

    template <class CostCalculator>
    template <class Sequence0, class Sequence1>
    Alignment AffineAligner<CostCalculator>::
    partial_align(Sequence0 const &seq0, Sequence1 const &seq1) const
    {
        using namespace detail;
        namespace m = Geert_cs_uu_nl;
        m_ccal.preprocess(seq0, seq1);
	size_t sz1 = boost::distance(seq0);
	size_t sz2 = boost::distance(seq1);
        m::Matrix<NwgCell> matrix(sz2+1, sz1+1, m::Uninit());
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

#endif // MUSALIGNA7GDNETYQI385MDKWOWIP
