CREATE DATABASE  IF NOT EXISTS `playdek` /*!40100 DEFAULT CHARACTER SET utf8 */;
USE `playdek`;
-- MySQL dump 10.13  Distrib 5.6.13, for Win32 (x86)
--
-- Host: 10.16.4.44    Database: playdek
-- ------------------------------------------------------
-- Server version	5.1.49-3-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `tournament_definition`
--

DROP TABLE IF EXISTS `tournament_definition`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `tournament_definition` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `game_id` int(11) DEFAULT NULL,
  `uuid` varchar(16) DEFAULT '0',
  `name_lookup_string` varchar(45) DEFAULT NULL,
  `announcement_lookup_string` varchar(45) DEFAULT NULL,
  `descrition_lookup_string` varchar(45) DEFAULT NULL,
  `begin_date` datetime DEFAULT NULL,
  `end_date` datetime DEFAULT NULL,
  `time_per_round` int(11) DEFAULT '3',
  `time_units_per_round` int(11) DEFAULT NULL,
  `num_players_per_instance` int(11) DEFAULT NULL,
  `icon` varchar(45) DEFAULT NULL,
  `image` varchar(45) DEFAULT NULL,
  `play_style` int(11) DEFAULT '1',
  `games_per_tournament_group` int(11) DEFAULT '1',
  `wins_to_finish_group` int(11) DEFAULT '1',
  `group_loses_to_count_as_tournament_loss` int(11) DEFAULT '0',
  `pd_points_earned_per_win` int(11) DEFAULT '0',
  `pd_points_earned_per_loss` int(11) DEFAULT '0',
  `pd_points_earned_per_participate` int(11) DEFAULT '0',
  `pd_points_earned_per_game_played` int(11) DEFAULT '0',
  `grouping_rule` int(11) DEFAULT NULL,
  `games_played_in_parallel` tinyint(4) DEFAULT '0',
  `are_losers_rematched` tinyint(4) DEFAULT '0',
  `game_version_minimum` decimal(1,0) DEFAULT NULL,
  `deck_limitations` varchar(45) DEFAULT NULL,
  `does_draft` tinyint(4) DEFAULT NULL,
  `notes` varchar(100) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=2 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2014-11-13 15:07:58
