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
-- Table structure for table `user_profile`
--

DROP TABLE IF EXISTS `user_profile`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `user_profile` (
  `user_id` int(11) NOT NULL,
  `address1` varchar(127) DEFAULT NULL,
  `address2` varchar(127) DEFAULT NULL,
  `city` varchar(45) DEFAULT NULL,
  `provence` varchar(45) DEFAULT NULL,
  `mail_code` varchar(12) DEFAULT NULL,
  `country` varchar(30) DEFAULT NULL,
  `marketing_opt_out` tinyint(1) DEFAULT '0',
  `screen_name` varchar(45) DEFAULT NULL,
  `gender` varchar(1) DEFAULT NULL,
  `mber_avatar` int(11) DEFAULT NULL,
  `home_phone` varchar(22) DEFAULT NULL,
  `alt_phone` varchar(22) DEFAULT NULL,
  `show_profile_gender` tinyint(1) DEFAULT '0',
  `admin_level` tinyint(1) DEFAULT '0',
  `show_win_loss_record` tinyint(1) DEFAULT '1',
  `time_zone` tinyint(1) DEFAULT '0',
  `account_create_product_id` int(11) DEFAULT '0',
  `motto` varchar(80) DEFAULT NULL,
  `display_online_status_to_others` tinyint(1) DEFAULT '1',
  `block_contact_invitations` tinyint(1) DEFAULT '0',
  `block_group_invitations` tinyint(1) DEFAULT '0',
  PRIMARY KEY (`user_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2014-11-13 15:07:47
