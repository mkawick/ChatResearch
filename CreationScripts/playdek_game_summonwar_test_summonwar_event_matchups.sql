CREATE DATABASE  IF NOT EXISTS `playdek_game_summonwar_test` /*!40100 DEFAULT CHARACTER SET latin1 */;
USE `playdek_game_summonwar_test`;
-- MySQL dump 10.13  Distrib 5.6.13, for Win32 (x86)
--
-- Host: 10.16.4.44    Database: playdek_game_summonwar_test
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
-- Table structure for table `summonwar_event_matchups`
--

DROP TABLE IF EXISTS `summonwar_event_matchups`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `summonwar_event_matchups` (
  `event_id` int(10) unsigned NOT NULL,
  `matchup_index` int(11) unsigned NOT NULL,
  `player_id_1` int(10) unsigned NOT NULL,
  `player_id_2` int(10) unsigned NOT NULL,
  `game_id` int(11) unsigned NOT NULL,
  `winner_id` int(10) unsigned DEFAULT NULL,
  `forfeit_flags` int(10) unsigned DEFAULT NULL,
  KEY `summonwar_event_matchups_ibfk_1` (`event_id`),
  CONSTRAINT `summonwar_event_matchups_ibfk_1` FOREIGN KEY (`event_id`) REFERENCES `summonwar_events` (`event_id`) ON DELETE CASCADE ON UPDATE NO ACTION
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2014-11-13 15:07:39
