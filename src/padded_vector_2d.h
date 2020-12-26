#ifndef PADDED_VECTOR_2D_H
#define PADDED_VECTOR_2D_H

#include <vector>

template <typename T>
class PaddedVector2D
{
public:
  PaddedVector2D(std::size_t n_cols, T m_padding_value, std::vector<T> raw)
    : m_raw{ std::move(raw) }
    , m_n_cols{ n_cols }
    , m_padding_value{ std::move(m_padding_value) }
  {}

  const T& at(int row, int column) const
  {
    if (row < 0 || row > rows() - 1 || column < 0 || column > cols() - 1) {
      return m_padding_value;
    } else {
      return m_raw[index(row, column)];
    }
  }

  T& at(int row, int column)
  {
    if (row < 0 || row > rows() - 1 || column < 0 || column > cols() - 1) {
      return m_padding_value;
    } else {
      return m_raw[index(row, column)];
    }
  }

  std::size_t rows() const
  {
    return m_raw.size() / m_n_cols;
  }

  std::size_t cols() const
  {
    return m_n_cols;
  }

  const std::vector<T>& raw() const
  {
    return m_raw;
  }

private:
  std::size_t index(int row, int column) const
  {
    return m_n_cols * row + column;
  };

  std::vector<T> m_raw;
  std::size_t m_n_cols;
  T m_padding_value;
};

#endif // PADDED_VECTOR_2D_H
