
#ifndef _H_SERVER_RATING_GLICKO2_
#define _H_SERVER_RATING_GLICKO2_

class RatingGlicko2 {
friend void RatingApplyGameResult( int player_count, RatingGlicko2 **pRatings, int *pScores );
private:
   double m_Rating;
   double m_Deviation;
   double m_Volatility;

public:
   RatingGlicko2();


   double   GetRating() const;
   double   GetDeviation() const;
   double   GetVolatility() const;

   unsigned short GetDisplayRating() const;

   void  Reset();

   void  SetRating( double rating );
   void  SetDeviation( double deviation );
   void  SetVolatility( double volatility );
};

void RatingApplyGameResult( int player_count, RatingGlicko2 **pRatings, int *pScores );


#endif



