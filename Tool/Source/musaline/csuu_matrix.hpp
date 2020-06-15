#ifndef CS_UU_NL_GEERT_2004_2_MATRIX_H
#define CS_UU_NL_GEERT_2004_2_MATRIX_H

#include <cstddef>
#include <memory>
#include <stdexcept>
// #include <cstdlib>
// #include <iostream>

namespace Geert_cs_uu_nl {

    // a matrix_util class takes care of translating matrix indices
    // (row and column indices and sizes) to indices in the vector where
    // the elements of such a matrix are stored.
    // Following interface should be supplied.
    //
    //	typedef std::size_t IndexType;
    //	Constructor(IndexType rows, IndexType cols) 
    //	void resize(IndexType rows, IndexType cols);
    //	bool is_valid_index(IndexType row, IndexType col) const;
    //	void assert_valid_index(IndexType row, IndexType col) const;
    //	IndexType rep_index(IndexType row, IndexType col) const;
    //	IndexType rep_size() const;
    //	IndexType row_size() const;
    //	IndexType column_size() const;


    using std::size_t;

    struct index_error: std::out_of_range {
        index_error(size_t row_idx, size_t col_idx, size_t row_sz, size_t col_sz)
            : out_of_range("Illegal (row, column) index"),
            m_ri(row_idx),m_ci(col_idx), m_rs(row_sz), m_cs(col_sz)
        {}
        size_t row_index() const {return m_ri;}
        size_t col_index() const {return m_ci;}
        size_t row_size() const {return m_rs;}
        size_t col_size() const {return m_cs;}
    private:
        size_t m_ri, m_ci, m_rs, m_cs;
    };

    struct General_cursor {
        typedef std::size_t IndexType;
        General_cursor(IndexType rows, IndexType cols)
            : 
#ifndef GJMATRIX_NO_CHECKS
                m_rows(rows),
#endif
                m_cols(cols),m_row(0),m_col(0),m_idx(0)
        {}
        General_cursor(IndexType rows, IndexType cols, IndexType row, IndexType col)
            :
#ifndef GJMATRIX_NO_CHECKS
                m_rows(rows),
#endif
                m_cols(cols),m_row(row),m_col(col),m_idx(row*cols+col)
        {}

        void inc_col()
        {
            ++m_col;
            ++m_idx;
        }

        void dec_col()
        {
            --m_col;
            --m_idx;
        }

        void inc_row()
        {
            ++m_row;
            m_idx+=m_cols;
        }

        void dec_row()
        {
            --m_row;
            m_idx-=m_cols;
        }

        IndexType row() const {return m_row;}
        IndexType col() const {return m_col;}
        IndexType index() const {
#ifndef GJMATRIX_NO_CHECKS
            if (m_row<0 || m_rows<=m_row || m_col<0 || m_cols<=m_col)
                throw index_error(m_col, m_col, m_rows, m_cols);
#endif
            return m_idx;
        }
        private:
#ifndef GJMATRIX_NO_CHECKS
        IndexType m_rows;
#endif
        IndexType m_cols, m_row, m_col, m_idx;
    };

    struct General_matrix_util {
        typedef std::size_t IndexType;
        typedef General_cursor CursorType;
        IndexType m_rows, m_cols;
        General_matrix_util(IndexType rows, IndexType cols)
            :m_rows(rows), m_cols(cols) 
        {}
        void resize(IndexType rows, IndexType cols)
        {
            m_rows = rows;
            m_cols = cols;
        }
        bool is_valid_index(IndexType row, IndexType col) const
        {
            return (row < m_rows) && (col < m_cols);
        }
        void assert_valid_index(IndexType row, IndexType col) const
        {
            if (! is_valid_index(row, col)) {
                throw index_error(row, col, m_rows, m_cols);
                // std::clog<<"index: ("<<row<<", "<<col<<"); bounds: ("
                //     <<m_rows<<", "<<m_cols<<")\n";
                // std::abort();
            }
        }
        IndexType rep_index(IndexType row, IndexType col) const
        { return m_cols*row+col;}
        IndexType rep_size() const
        { return m_rows*m_cols;}
        IndexType row_size() const {return m_rows;}
        IndexType column_size() const {return m_cols;}
    };

    struct Square_cursor {
        typedef std::size_t IndexType;
        Square_cursor(IndexType rows, IndexType cols)
            : m_n(rows),m_row(0),m_col(0),m_idx(0)
        {
            if (rows != cols)
                throw std::invalid_argument("rows and cols differ");
        }
        Square_cursor(IndexType rows, IndexType cols, IndexType row, IndexType col)
            : m_n(rows),m_row(row),m_col(col),m_idx(row*cols+col)
        {
            if (rows != cols)
                throw std::invalid_argument("rows and cols differ");
        }

        void inc_col()
        {
            ++m_col;
            ++m_idx;
        }

        void dec_col()
        {
            --m_col;
            --m_idx;
        }

        void inc_row()
        {
            ++m_row;
            m_idx+=m_n;
        }

        void dec_row()
        {
            --m_row;
            m_idx-=m_n;
        }

        IndexType row() const {return m_row;}
        IndexType col() const {return m_col;}
        IndexType index() const {
#ifndef GJMATRIX_NO_CHECKS
            if (m_row<0 || m_n<=m_row || m_col<0 || m_n<=m_col)
                throw index_error(m_col, m_col, m_n, m_n);
#endif
            return m_idx;
        }
        private:
        IndexType m_n, m_row, m_col, m_idx;
    };

    struct Square_matrix_util {
        typedef std::size_t IndexType;
        typedef Square_cursor CursorType;
        Square_matrix_util(IndexType rows, IndexType cols)
            :m_n(rows) 
        {
            if (rows != cols)
                throw std::invalid_argument("rows and cols differ");
        }
        IndexType m_n;
        void resize(IndexType rows, IndexType cols)
        {
            if (rows != cols)
                throw std::invalid_argument("rows and cols differ");
            m_n = rows;
        }
        bool is_valid_index(IndexType row, IndexType col) const
        {
            return (row < m_n) && (col < m_n);
        }
        void assert_valid_index(IndexType row, IndexType col) const
        {
            if (!is_valid_index(row, col))
                throw index_error(row, col, m_n, m_n);
        }
        IndexType rep_index(IndexType row, IndexType col) const
        { return m_n*row+col;}
        IndexType rep_size() const
        { return m_n*m_n;}
        IndexType row_size() const {return m_n;}
        IndexType column_size() const {return m_n;}
    };

    struct Triangular_URM_cursor {
        typedef std::size_t IndexType;
        Triangular_URM_cursor(IndexType rows, IndexType cols)
            : m_n(rows),m_row(0),m_col(1),m_idx(0)
        {
            if (rows != cols)
                throw std::invalid_argument("rows and cols differ");
        }
        Triangular_URM_cursor(IndexType rows, IndexType cols, IndexType row, IndexType col)
            : m_n(rows),m_row(row),m_col(col),m_idx(row*cols-row*(row+3)/2+col-1)
        {
            if (rows != cols)
                throw std::invalid_argument("rows and cols differ");
        }
        void inc_col()
        {
            ++m_col;
            ++m_idx;
        }

        void dec_col()
        {
            --m_col;
            --m_idx;
        }

        void inc_row()
        {
            m_idx +=  m_n-m_row-2;
            ++m_row;
        }

        void dec_row()
        {
            --m_row;
            m_idx -=  m_n-m_row-2;
        }

        IndexType row() const {return m_row;}
        IndexType col() const {return m_col;}
        IndexType index() const {
#ifndef GJMATRIX_NO_CHECKS
            if (m_row<0 || m_n<=m_row || m_col<0 || m_n<=m_col)
                throw index_error(m_col, m_col, m_n, m_n);
#endif
            return m_idx;
        }
        private:
        IndexType m_n, m_row, m_col, m_idx;
    };

    struct Triangular_URM_matrix_util {
        typedef std::size_t IndexType;
        typedef Triangular_URM_cursor CursorType;
        Triangular_URM_matrix_util(IndexType rows, IndexType cols)
            :m_n(rows) 
        {
            if (rows != cols)
                throw std::invalid_argument("rows and cols differ");
        }
        IndexType m_n;
        void resize(IndexType rows, IndexType cols)
        {
            if (rows != cols)
                throw std::invalid_argument("rows and cols differ");
            m_n = rows;
        }
        bool is_valid_index(IndexType row, IndexType col) const
        {
            return (0 <= row) && (row < col) && (col < m_n);
        }
        void assert_valid_index(IndexType row, IndexType col) const
        {
            if (!is_valid_index(row, col))
                throw index_error(row, col, m_n, m_n);
        }
        IndexType rep_index(IndexType row, IndexType col) const
        { return m_n*row-row*(row+3)/2+col-1;}
        IndexType rep_size() const
        { return m_n*(m_n-1)/2;}
        IndexType row_size() const {return m_n;}
        IndexType column_size() const {return m_n;}
    };

    class Uninit {};

    // Base_matrix combines the index_translating MatrixUtil class
    // and a representation.
    template <typename ElemType, typename MatrixUtil>
        class Base_matrix {
            MatrixUtil m_mat_util;
            typedef std::allocator<ElemType> Alloc;
            typename Alloc::pointer m_rep;
          public:
            typedef std::size_t IndexType;
            typedef typename MatrixUtil::CursorType CursorType;
            ~Base_matrix()
            {
                if (m_rep==0)
                    return;
                size_t size = m_mat_util.rep_size();
                typename Alloc::pointer cur(m_rep),
                         end(m_rep+size);
                Alloc al;
                for ( ; cur!=end; ++cur) {
                    al.destroy(cur);
                }
                al.deallocate(m_rep, size);
            }

            Base_matrix(Base_matrix const &o)
                :m_mat_util(o.m_mat_util)
            {
                int size = o.m_mat_util.rep_size();
                if (size==0) {
                    m_rep=0;
                } else {
                    Alloc al;
                    m_rep = al.allocate(size);
                    typename Alloc::pointer cur(m_rep), end(m_rep+size),
                             ocur(o.m_rep);
                    for ( ; cur!=end; ++cur, ++ocur) {
                        al.construct(cur,*ocur);
                    }
                }
            }
            void swap(Base_matrix &o)
            {
                using std::swap;
                swap(m_mat_util, o.m_mat_util);
                swap(m_rep, o.m_rep);
            }
            Base_matrix operator=(Base_matrix const &o)
            {
                Base_matrix(o).swap(*this);
                return *this;
            }
            Base_matrix()
                :m_mat_util(0,0), m_rep(0) {}
            Base_matrix(IndexType rows,IndexType cols, Uninit)
                : m_mat_util(rows,cols) 
            {
                int size = m_mat_util.rep_size();
                if (size==0) {
                    m_rep=0;
                } else {
                    Alloc al;
                    m_rep = al.allocate(size);
                }
            }
            Base_matrix(IndexType rows,IndexType cols, ElemType t = ElemType())
                : m_mat_util(rows,cols) 
            {
                int size = m_mat_util.rep_size();
                if (size==0) {
                    m_rep=0;
                } else {
                    Alloc al;
                    m_rep = al.allocate(size);
                    typename Alloc::pointer cur(m_rep), end(m_rep+size);
                    for ( ; cur!=end; ++cur) {
                        al.construct(cur,t);
                    }
                }
            }
            // The templated version below collides easily with the constructor above
            //		template <typename Iter>
            //		Base_matrix(IndexType rows, IndexType cols, Iter begin)
            //		: m_mat_util(0, 0) 
            //		{
            //			re_init_iter(rows,cols,begin);
            //		}
            IndexType row_size() const {return m_mat_util.row_size();}
            IndexType column_size() const {return m_mat_util.column_size();}
            ElemType & at(IndexType row, IndexType col)
            { 
                if (!m_mat_util.is_valid_index(row,col)) {
                    throw index_error(row, col,
                            m_mat_util.row_size(), m_mat_util.col_size());
                }
                return m_rep[m_mat_util.rep_index(row,col)];
            }
            ElemType const &at(IndexType row, IndexType col) const
            { 
                if (!m_mat_util.is_valid_index(row,col)) {
                    throw index_error(row, col,
                            m_mat_util.row_size(), m_mat_util.col_size());
                }
                return m_rep[m_mat_util.rep_index(row,col)];
            }
            ElemType & atn(IndexType row, IndexType col)
            { 
                return m_rep[m_mat_util.rep_index(row,col)];
            }
            ElemType const &atn(IndexType row, IndexType col) const
            { 
                return m_rep[m_mat_util.rep_index(row,col)];
            }
            ElemType & operator()(IndexType row, IndexType col)
            { 
#ifndef GJMATRIX_NO_CHECKS
                m_mat_util.assert_valid_index(row,col);
#endif
                return m_rep[m_mat_util.rep_index(row,col)];
            }
            ElemType const &operator()(IndexType row, IndexType col) const
            { 
#ifndef GJMATRIX_NO_CHECKS
                m_mat_util.assert_valid_index(row,col);
#endif
                return m_rep[m_mat_util.rep_index(row,col)];
            }
            ElemType &operator()(CursorType const &crs)
            {
#ifndef GJMATRIX_NO_CHECKS
                m_mat_util.assert_valid_index(crs.row(),crs.col());
#endif
                return m_rep[crs.index()];
            }
            ElemType const &operator()(CursorType const &crs) const
            {
#ifndef GJMATRIX_NO_CHECKS
                m_mat_util.assert_valid_index(crs.row(),crs.col());
#endif
                return m_rep[crs.index()];
            }
            CursorType get_cursor(IndexType rows,IndexType cols) const
            { return CursorType(row_size(), column_size(),rows,cols);}
            CursorType get_cursor() const
            { return CursorType(row_size(), column_size());}
            void re_init_n(IndexType rows,IndexType cols, ElemType t = ElemType())
            { 
                m_mat_util.resize(rows,cols); 
                m_rep.clear(); 
                m_rep.resize(m_mat_util.rep_size(),t);
            }
            template <typename Iter>
                void re_init_iter(IndexType rows,IndexType cols, Iter begin)
                {
                    m_mat_util.resize(rows,cols);
                    IndexType const elemCount = m_mat_util.rep_size();
                    m_rep.clear();
                    m_rep.reserve(elemCount);
                    for (IndexType i=0; i<elemCount; ++i) {
                        m_rep.push_back(*begin);
                        ++begin;
                    }
                }
        };


    template <typename ElemType>
        class Triangular_URM_matrix: public Base_matrix<ElemType, Triangular_URM_matrix_util>
    {
        typedef Base_matrix<ElemType, Triangular_URM_matrix_util> Base_;
        typedef std::size_t IndexType;
        public:
        Triangular_URM_matrix() {}
        Triangular_URM_matrix(IndexType n)
            : Base_(n,n,ElemType()) {}
        Triangular_URM_matrix(IndexType n, Uninit u)
            : Base_(n,n,u) {}
        Triangular_URM_matrix(IndexType n, ElemType t)
            : Base_(n,n,t) {}
        //		template <typename Iter>
        //			Triangular_URM_matrix(IndexType n, Iter begin): Base_(n,n,begin) {}
        void re_init_n(IndexType rows, ElemType t = ElemType())
        { 
            Base_::re_init_n(rows,rows,t); 
        }
        template <typename Iter>
            void re_init_iter(IndexType rows, Iter begin)
            { 
                Base_::re_init_iter(rows,rows,begin); 
            }
    };

    template <typename ElemType>
        class Square_matrix: public Base_matrix<ElemType, Square_matrix_util>
    {
        typedef std::size_t IndexType;
        typedef Base_matrix<ElemType, Square_matrix_util> Base_;
        public:
        Square_matrix() {}
        Square_matrix(IndexType n)
            : Base_(n,n,ElemType()) {}
        Square_matrix(IndexType n, Uninit u )
            : Base_(n,n,u) {}
        Square_matrix(IndexType n, ElemType t )
            : Base_(n,n,t) {}
        //		template <typename Iter>
        //			Square_matrix(IndexType n, Iter begin): Base_(n,n,begin) {}
        void re_init_n(IndexType rows, ElemType t = ElemType())
        { 
            Base_::re_init_n(rows,rows,t); 
        }
        template <typename Iter>
            void re_init_iter(IndexType rows, Iter begin)
            { 
                Base_::re_init_iter(rows,rows,begin); 
            }
    };

    template <typename ElemType>
        class Matrix: public Base_matrix<ElemType, General_matrix_util>
    {
        typedef std::size_t IndexType;
        typedef Base_matrix<ElemType, General_matrix_util> Base_;
        public:
        Matrix() {}
        Matrix(IndexType rows, IndexType cols)
            : Base_(rows,cols,ElemType()) {}
        Matrix(IndexType rows, IndexType cols, ElemType t )
            : Base_(rows,cols,t) {}
        Matrix(IndexType rows, IndexType cols, Uninit u )
            : Base_(rows,cols,u) {}
        //		template <typename Iter>
        //			Matrix(IndexType rows, IndexType cols, Iter begin): Base_(rows,cols,begin) {}
        void re_init_n(IndexType rows, IndexType cols, ElemType t = ElemType())
        { 
            Base_::re_init_n(rows,cols,t); 
        }
        template <typename Iter>
            void re_init_iter(IndexType rows, IndexType cols, Iter begin)
            { 
                Base_::re_init_iter(rows,cols,begin); 
            }
    };

}

// vim:set shiftwidth=4 softtabstop=4 expandtab cindent:

#endif
