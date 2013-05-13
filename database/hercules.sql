-- MySQL dump 10.11
--
-- Host: localhost    Database: hercules
-- ------------------------------------------------------
-- Server version	5.0.45

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
-- Table structure for table `configuration`
--

DROP TABLE IF EXISTS `configuration`;
CREATE TABLE `configuration` (
  `name` varchar(100) NOT NULL,
  `value` varchar(100) default NULL,
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `hosts`
--

DROP TABLE IF EXISTS `hosts`;
CREATE TABLE `hosts` (
  `host` char(8) NOT NULL,
  `description` varchar(30) default NULL,
  `status` varchar(30) default NULL,
  `server` varchar(40) default NULL,
  `tn3270port` int(11) default NULL,
  `readerport` int(11) default NULL,
  `sysreaderport` int(11) default NULL,
  `printroot` varchar(100) default NULL,
  `readerroot` varchar(100) default NULL,
  PRIMARY KEY  (`host`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `jobinput`
--

DROP TABLE IF EXISTS `jobinput`;
CREATE TABLE `jobinput` (
  `jobnum` int(7) unsigned NOT NULL auto_increment,
  `user` varchar(15) default NULL,
  `host` varchar(8) default NULL,
  `account` varchar(8) default NULL,
  `userjobname` varchar(8) default NULL,
  `userheader` varchar(800) default NULL,
  `header` varchar(800) default NULL,
  `status` varchar(20) default NULL,
  `filepointer` varchar(200) default NULL,
  `printnum` int(11) default NULL,
  `submitted` datetime default NULL,
  `retainuntil` datetime default NULL,
  PRIMARY KEY  (`jobnum`),
  UNIQUE KEY `jobinput_user_index` (`user`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `printoutput`
--

DROP TABLE IF EXISTS `printoutput`;
CREATE TABLE `printoutput` (
  `printnum` int(10) unsigned NOT NULL auto_increment,
  `jobname` varchar(8) default NULL,
  `user` varchar(8) default NULL,
  `host` varchar(8) default NULL,
  `created` datetime default NULL,
  `retainuntil` datetime default NULL,
  `jobclass` varchar(1) default NULL,
  `jesjobnum` int(11) default NULL,
  `printoutput` longtext,
  PRIMARY KEY  (`printnum`),
  KEY `printoutput_user_index` (`user`),
  KEY `printoutput_findjob_index` (`jobname`,`user`,`host`)
) ENGINE=MyISAM AUTO_INCREMENT=126 DEFAULT CHARSET=latin1;

--
-- Table structure for table `useraccounts`
--

DROP TABLE IF EXISTS `useraccounts`;
CREATE TABLE `useraccounts` (
  `user` varchar(15) NOT NULL,
  `host` varchar(8) NOT NULL,
  `account` varchar(8) NOT NULL,
  `password` varchar(8) default NULL,
  PRIMARY KEY  (`user`,`host`,`account`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `userhosts`
--

DROP TABLE IF EXISTS `userhosts`;
CREATE TABLE `userhosts` (
  `user` varchar(15) NOT NULL,
  `host` varchar(8) NOT NULL,
  `adminflag` int(11) NOT NULL,
  PRIMARY KEY  (`user`,`host`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2009-05-04  8:32:50
