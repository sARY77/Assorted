#include <array>
#include <iostream>
#include <set>
#include <utility>

class Board
{
public:
   enum class Type : int
   {
      Horizontal,
      Vertical,
      Square
   };

   int Get(Type type, size_t index1, size_t index2) const noexcept
   {
      return const_cast<Board*>(this)->Get(type, index1, index2);
   }

   int& Get(Type type, size_t index1, size_t index2) noexcept
   {
      switch (type)
      {
      case Type::Horizontal:
         return m_board[index1][index2];
      case Type::Vertical:
         return m_board[index2][index1];
      default:
         return m_board[GetSquareRow(index1, index2)][GetSquareColumn(index1, index2)];
      }
   }

   bool Has(Type type, size_t index1, int number) const noexcept
   {
      for (size_t index2 = 0; index2 < 9; ++index2)
         if (Get(type, index1, index2) == number)
            return true;
      return false;
   }

   bool IsSingleCandidate(const std::array<std::array<std::set<int>, 9>, 9>& candidates, size_t row, size_t column, size_t square, int number) const noexcept
   {
      if (candidates[row][column].size() == 1)
         return true;
      size_t rowCount = 0, columnCount = 0, squareCount = 0;
      for (size_t index = 0; index < 9; ++index)
      {
         rowCount += candidates[row][index].count(number);
         columnCount += candidates[index][column].count(number);
         squareCount += candidates[GetSquareRow(square, index)] [GetSquareColumn(square, index)].count(number);
      }
      return rowCount == 1 || columnCount == 1 || squareCount == 1;
   }

   void SetPossible() noexcept
   {
      for (;;)
      {
         std::array<std::array<std::set<int>, 9>, 9> candidates{};
         std::array<std::array<std::multiset<int>, 9>, 3> counts{};
         for (int number = 1; number <= 9; ++number)
            for (size_t row = 0; row < 9; ++row)
               if (!Has(Type::Horizontal, row, number))
                  for (size_t column = 0; column < 9; ++column)
                     if (m_board[row][column] == 0)
                        if (!Has(Type::Vertical, column, number))
                        {
                           const size_t square = GetSquare(row, column);
                           if (!Has(Type::Square, square, number))
                           {
                              candidates[row][column].insert(number);
                              counts[static_cast<std::underlying_type_t<Type>>(Type::Horizontal)][row].insert(number);
                              counts[static_cast<std::underlying_type_t<Type>>(Type::Vertical)][column].insert(number);
                              counts[static_cast<std::underlying_type_t<Type>>(Type::Square)][square].insert(number);
                           }
                        }
         bool isFound = false;
         for (size_t row = 0; row < 9; ++row)
            for (size_t column = 0; column < 9; ++column)
            {
               bool isFoundOnce = false;
               const size_t square = GetSquare(row, column);
               for (const int number : candidates[row][column])
                  if (//candidates[row][column].size() == 1 ||
                     counts[static_cast<std::underlying_type_t<Type>>(Type::Horizontal)][row].count(number) == 1 ||
                     counts[static_cast<std::underlying_type_t<Type>>(Type::Vertical)][column].count(number) == 1 ||
                     counts[static_cast<std::underlying_type_t<Type>>(Type::Square)][square].count(number) == 1)
                  {
                     isFound = true;
                     if (isFoundOnce)
                        std::cout << "Multiple numbers error\n";
                     isFoundOnce = true;
                     std::cout << "[" << row << "][" << column << "] = " << number << std::endl;
                     m_board[row][column] = number;
                     Verify();
                  }
            }
         if (!isFound)
            break;
      }
   }

private:
   void Verify() const noexcept
   {
      std::array<std::array<std::multiset<int>, 9>, 3> counts{};
      for (size_t row = 0; row < 9; ++row)
         for (size_t column = 0; column < 9; ++column)
         {
            const int number = m_board[row][column];
            if (number != 0)
            {
               const size_t square = GetSquare(row, column);
               counts[static_cast<std::underlying_type_t<Type>>(Type::Horizontal)][row].insert(number);
               counts[static_cast<std::underlying_type_t<Type>>(Type::Vertical)][column].insert(number);
               counts[static_cast<std::underlying_type_t<Type>>(Type::Square)][square].insert(number);
            }
         }
      for (int number = 1; number <= 9; ++number)
         for (Type type : { Type::Horizontal, Type::Vertical, Type::Square })
            for (size_t index1 = 0; index1 < 9; ++index1)
               if (counts[static_cast<std::underlying_type_t<Type>>(type)][index1].count(number) > 1)
                  std::cout << "Verify error\n";
   }

   constexpr size_t GetSquare(size_t row, size_t column) const noexcept
   {
      return (row / 3) * 3 + column / 3;
   }

   constexpr size_t GetSquareRow(size_t square, size_t index) const noexcept
   {
      return (square / 3) * 3 + index / 3;
   }

   constexpr size_t GetSquareColumn(size_t square, size_t index) const noexcept
   {
      return (square % 3) * 3 + index % 3;
   }

   std::array<std::array<int, 9>, 9> m_board
   {
       std::array<int, 9>{ 0, 7, 0, 0, 0, 0, 0, 5, 3 },
       std::array<int, 9>{ 0, 0, 8, 0, 0, 0, 0, 0, 0 },
       std::array<int, 9>{ 0, 4, 6, 0, 1, 3, 0, 0, 0 },
       std::array<int, 9>{ 0, 0, 0, 3, 0, 2, 0, 0, 6 },
       std::array<int, 9>{ 0, 0, 5, 0, 8, 0, 4, 0, 0 },
       std::array<int, 9>{ 7, 0, 0, 5, 0, 4, 0, 0, 0 },
       std::array<int, 9>{ 0, 0, 0, 9, 7, 0, 8, 6, 0 },
       std::array<int, 9>{ 0, 0, 0, 0, 0, 0, 9, 0, 0 },
       std::array<int, 9>{ 8, 6, 0, 0, 0, 0, 0, 4, 0 }
   };
};

int main() noexcept
{
   Board board;
   board.SetPossible();
   return 0;
}
