#ifndef KENDALL_H
#define KENDALL_H

#include <algorithm>
#include <iostream>
#include <vector>

using std::make_pair;
using std::min;
using std::pair;
using std::sqrt;
using std::vector;

/**
 * @brief Calculates the kendal correlation accounting for ties.
 *
 * @param x First feature vector.
 * @param y Second feature vector.
 *
 * @return Kendall correlation [-1,1].
 */
double kendallCorrelation(const vector<double>& x,const vector<double>& y)
{
  //make array of pairs and sort by X (and Y if X have ties)
  vector<pair<double, double>> pairs;
  pairs.reserve(x.size());
  for(unsigned long long i = 0; i < x.size(); i++)
    pairs.emplace_back(make_pair(x[i], y[i]));
  std::sort(pairs.begin(), pairs.end(),
      [](const pair<double, double>& p1, const pair<double, double>& p2)
      {
        //note: p1.second < p2.second instead of p1.second <= p2.second is used
        //because std::sort requires strict weak ordering
        return (p1.first == p2.first ? p1.second < p2.second : p1.first < p2.first);
      });


  //pass the vector and count pairs having same X or same X and Y
  unsigned long long sameX = 0;//total pairs with same X
  unsigned long long sameXY = 0;//total pairs with same XY
  unsigned long long consecutiveSameX = 1;//current streak of pairs with same X
  unsigned long long consecutiveSameXY = 1;//current streak of pairs with same XY

  for(unsigned long long i = 1; i < pairs.size(); i++)
  {
    //if same X increment the current same X streak
    if(pairs[i].first == pairs[i-1].first)
    {
      consecutiveSameX++;
      /*
       * if also same Y increment the current same XY streak
       * otherwise update the same XY counter and reset the
       * streak to 1
       * */
      if(pairs[i].second == pairs[i-1].second)
        consecutiveSameXY++;
      else
      {
        // (n * (n - 1) )/2
        sameXY += (consecutiveSameXY * (consecutiveSameXY - 1)) / 2;
        consecutiveSameXY = 1;
      }
    }
    else
    {
      /*
       * if the pairs have different X values update the sameX and
       * sameXY counters and reset the X and XY streaks to 1
       * */
      sameX += (consecutiveSameX * (consecutiveSameX - 1)) / 2;
      consecutiveSameX = 1;

      sameXY += (consecutiveSameXY * (consecutiveSameXY - 1)) / 2;
      consecutiveSameXY = 1;
    }
  }
  //(needed if all the values are equal)
  sameX += (consecutiveSameX * (consecutiveSameX - 1)) / 2;
  sameXY += (consecutiveSameXY * (consecutiveSameXY -1)) / 2;


  /* sort the pairs by Y and get the number of swaps neeeded
   * to sort them by Y, since they were previously sorted
   * by X this will tell us the number of discording pairs
   */
  unsigned long long discording = 0;
  vector<pair<double, double>> holder(pairs.size());
  /*
  non recursive merge sort
  start from chunks of size 1 to n, merge (and count swaps)
  */
  for(unsigned long chunk = 1; chunk < pairs.size(); chunk *= 2)
  {
      //take 2 sorted chunks and make them one sorted chunk
      for(unsigned long startChunk = 0; startChunk < pairs.size(); startChunk += 2 * chunk)
      {
          //start and end of the left half
          unsigned long startLeft = startChunk;
          unsigned long endLeft = min(startLeft + chunk, pairs.size());

          //start and end of the right half
          unsigned long startRight = endLeft;
          unsigned long endRight = min(startRight + chunk, pairs.size());

          //merge the 2 halfs
          //index is used to point to the right place in the holder array
          unsigned long index = startLeft;
          for(;startLeft < endLeft && startRight < endRight; index++)
          {
              /*
              if the pairs (ordered by X) discord when checked by Y
              increment the number of discording pairs by 1 for each
              remaining pair on the left half, because if the pair on the right
              half discords with the pair on the left half it surely discords
              with all the remaining pairs on the left half, since they all
              have a Y greater than the Y of the left half pair currently
              being checked
              */
              if(pairs[startLeft].second > pairs[startRight].second)
              {
                  holder[index] = pairs[startRight];
                  startRight++;
                  discording += endLeft - startLeft;
              }
              else
              {
                  holder[index] = pairs[startLeft];
                  startLeft++;
              }
          }

          /*
          if the left half is over there are no more discording pairs in this
          chunk, the remaining pairs in the right half can be copied
          */
          for(;startRight < endRight; startRight++, index++)
              holder[index] = pairs[startRight];
          /*
          if the right half is over (and the left one is not) all the
          discording pairs have been accounted for already
          */
          for(;startLeft < endLeft; startLeft++, index++)
              holder[index] = pairs[startLeft];
      }
      pairs.swap(holder);
 }


 //pass the vector and count pairs having same Y
 unsigned long long sameY = 0;//counter
 unsigned long long consecutiveSameY = 1;//current streak
 for(unsigned long long i = 1; i < pairs.size(); i++)
 {
   if(pairs[i].second == pairs[i-1].second)
     consecutiveSameY++;
   else
   {
     sameY += (consecutiveSameY * (consecutiveSameY - 1ull)) / 2ull;
     consecutiveSameY = 1;
   }
 }
 sameY += (consecutiveSameY * (consecutiveSameY - 1ull)) / 2ull;

 //return the correlation
 unsigned long long totalPairs = ((pairs.size() * (pairs.size() - 1ull)) / 2ull);
 /*
  * concordant pairs - discorant pairs, having the sameX or sameY count
  * as a discording pair because the other value (Y or X) could be different,
  * to account for the fact that pairs having the sameX might have the same Y
  * sameXY is added back, the rest of the discording pairs has been calculated
  * during the merge sort ( - discording)
  * */
 long long num = totalPairs - sameX - sameY + sameXY - 2ull * discording;
 /*
  * squart root of
  * (pairs not tied in X) * (pairs not tied in Y)
  * */
 long double den = sqrt((long double)(totalPairs - sameX) * (totalPairs - sameY));
 return (den == 0.0)? (sameX == sameY? 1.0 : 0.0) : num/den;
}
#endif
