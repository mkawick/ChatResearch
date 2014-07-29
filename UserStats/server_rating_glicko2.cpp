
#include "server_rating_glicko2.h"

#include <cmath>

static const double kPISquared         = 9.86960440108935861883;

static double g( double deviation )
{
   return 1.0 / (sqrt(1.0 + (3.0 * deviation * deviation) / kPISquared));
}

static double E( double rating, double rating_opponent, double deviation_opponent )
{
   return 1.0 / (1.0 + exp(-g(deviation_opponent)*(rating-rating_opponent)));
}


static const double kRatingDefault      = 1500.0;
static const double kDeviationDefault   = 350.0;
static const double kVolatilityDefault  = 0.06;

static const double kRatingBase         = 1500.0;
static const double kRatingScale        = 173.7178;

static const double kDVolatility        = 0.3;


RatingGlicko2::RatingGlicko2()
   : m_Rating( 0.0 ),
      m_Deviation( 0.0 ),
      m_Volatility( 0.0 )
{
}

double RatingGlicko2::GetRating() const
{
   return kRatingBase + (m_Rating * kRatingScale);
}

double RatingGlicko2::GetDeviation() const
{
   return m_Deviation * kRatingScale;
}

double RatingGlicko2::GetVolatility() const
{
   return m_Volatility;
}

unsigned short RatingGlicko2::GetDisplayRating() const
{
   double rating = GetRating();
   return (unsigned short)(rating + 0.5);
}

void RatingGlicko2::Reset()
{
   SetRating( kRatingDefault );
   SetDeviation( kDeviationDefault );
   SetVolatility( kVolatilityDefault );
}

void RatingGlicko2::SetRating( double rating )
{
   m_Rating = (rating - kRatingBase) / kRatingScale;
}

void RatingGlicko2::SetDeviation( double deviation )
{
   m_Deviation = deviation / kRatingScale;
}

void RatingGlicko2::SetVolatility( double volatility )
{
   m_Volatility = volatility;
}








void RatingApplyGameResult( int player_count, RatingGlicko2 **pRatings, int *pScores )
{
   static float s_PlayerCountAdjustments[] = { 1.0f/1.0f, 1.0f/1.26f, 1.0f/1.42f, 1.0f/1.54f }; 
   //float player_count_adjustment = 1.0f - (((float)player_count-2) * 0.16f);
   float player_count_adjustment = s_PlayerCountAdjustments[player_count-2];

   double update_rating[5];
   double update_deviation[5];
   double update_volatility[5];

   for( int p = 0; p < player_count; ++p )
   {
      RatingGlicko2 *pUpdateRating = pRatings[p];
      update_rating[p] = pUpdateRating->m_Rating;
      update_deviation[p] = pUpdateRating->m_Deviation;
      update_volatility[p] = pUpdateRating->m_Volatility;
   }

   for( int p = 0; p < player_count; ++p )
   {
      const double updateDeviationSquared = update_deviation[p] * update_deviation[p];
      const double updateVolatilitySquared = update_volatility[p] * update_volatility[p];

      double new_rating = 0.0;

      double variance = 0.0;
      //double delta = 0.0;

      for( int i = 0; i < player_count; ++i )
      {
         if( i == p )
         {
            continue;
         }
         double result = 0.0;
         if( pScores[p] == pScores[i] )
         {
            result = 0.5f;
         }
         else if( pScores[p] > pScores[i] )
         {
            result = 1.0f;
         }

         double g_i = g(pRatings[i]->m_Deviation);
         double E_i = E(update_rating[p],pRatings[i]->m_Rating,pRatings[i]->m_Deviation);
         variance += g_i * g_i * E_i * (1.0 - E_i);

         new_rating += g_i * (result - E_i);
     }
      variance = 1.0 / variance;
      const double delta = new_rating * variance;


      const double a = log( updateVolatilitySquared );
      double x = 0.0;
      double x_new = a;
      while( fabs(x-x_new) > 0.000001 )
      {
         x = x_new;
         const double exp_x = exp(x);
         const double d = updateDeviationSquared + variance + exp_x;
         const double h1 = -(x-a)/(kDVolatility*kDVolatility) - (0.5*exp_x)/d + 0.5*exp(x)*(delta/d)*(delta/d);
         const double h2 = -1.0/(kDVolatility*kDVolatility)
            - 0.5*exp_x*(updateDeviationSquared+variance)/(d*d)
            + 0.5*(delta*delta)*exp_x*(updateDeviationSquared+variance-exp_x)/(d*d*d);
         x_new = x - h1/h2;
      }
      const double new_volatility = exp(x_new / 2.0);

      const double pre_deviation = sqrt(updateDeviationSquared + new_volatility*new_volatility);

      const double new_deviation = 1.0 / (sqrt(1.0/(pre_deviation*pre_deviation) + 1.0/variance));

      new_rating *= new_deviation * new_deviation;
      new_rating += update_rating[p];

      update_rating[p] += (new_rating - update_rating[p]) * player_count_adjustment;
      update_deviation[p] += (new_deviation - update_deviation[p]) * player_count_adjustment;
      update_volatility[p] += (new_volatility - update_volatility[p]) * player_count_adjustment;
   }

   for( int p = 0; p < player_count; ++p )
   {
      RatingGlicko2 *pUpdateRating = pRatings[p];
      pUpdateRating->m_Rating = update_rating[p];
      pUpdateRating->m_Deviation = update_deviation[p];
      pUpdateRating->m_Volatility = update_volatility[p];
   }

}
