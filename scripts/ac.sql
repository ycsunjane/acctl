-- phpMyAdmin SQL Dump
-- version 4.0.4
-- http://www.phpmyadmin.net
--
-- 主机: 127.0.0.1
-- 生成日期: 2014 年 10 月 25 日 14:42
-- 服务器版本: 5.5.32
-- PHP 版本: 5.4.16

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- 数据库: `ac`
--
CREATE DATABASE IF NOT EXISTS `ac` DEFAULT CHARACTER SET utf8 COLLATE utf8_unicode_ci;
USE `ac`;

-- --------------------------------------------------------

--
-- 表的结构 `resource`
--

CREATE TABLE IF NOT EXISTS `resource` (
  `ip_start` varchar(15) COLLATE utf8_unicode_ci,
  `ip_end`   varchar(15) COLLATE utf8_unicode_ci,
  `ip_mask`  varchar(15) COLLATE utf8_unicode_ci 
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- 表的结构 `node`
--

CREATE TABLE IF NOT EXISTS `node` (
  `hostname` varchar(32) COLLATE utf8_unicode_ci DEFAULT NULL COMMENT '设备自身的名称',
  `time_first` datetime DEFAULT NULL COMMENT '设备第一报告时间',
  `time` datetime DEFAULT NULL COMMENT 'Time of last checkin',
  `latitude` varchar(20) COLLATE utf8_unicode_ci DEFAULT NULL,
  `longitude` varchar(20) COLLATE utf8_unicode_ci DEFAULT NULL,
  `uptime` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL COMMENT 'ROBIN',
  `memfree` varchar(20) COLLATE utf8_unicode_ci DEFAULT NULL COMMENT 'ROBIN',
  `cpu` varchar(16) COLLATE utf8_unicode_ci DEFAULT NULL,
  `device_down` tinyint(1) NOT NULL DEFAULT '0' COMMENT '标示设备是否离线，并且发送短信了',
  `wan_iface` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL,
  `wan_ip` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL,
  `wan_mac` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL,
  `wan_gateway` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL,
  `wifi_iface` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL,
  `wifi_ip` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL,
  `wifi_mac` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL,
  `wifi_ssid` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL,
  `wifi_encryption` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL COMMENT '无线加密方式',
  `wifi_key` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL,
  `wifi_channel_mode` varchar(16) COLLATE utf8_unicode_ci DEFAULT NULL COMMENT 'wifi终端采用的信道模式，自动还是手动',
  `wifi_channel` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL,
  `wifi_signal` varchar(16) COLLATE utf8_unicode_ci DEFAULT NULL,
  `lan_iface` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL,
  `lan_mac` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL,
  `lan_ip` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL,
  `wan_bup` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL,
  `wan_bup_sum` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL,
  `wan_bdown` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL,
  `wan_bdown_sum` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL,
  `firmware` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL,
  `firmware_revision` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL,
  `online_user_num` int(11) DEFAULT '0',
  UNIQUE KEY `wan_mac` (`wan_mac`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='node database' AUTO_INCREMENT=2 ;

-- --------------------------------------------------------

--
-- 表的结构 `node_default`
--

CREATE TABLE IF NOT EXISTS `node_default` (
  `profile` varchar(12) COLLATE utf8_unicode_ci NOT NULL COMMENT '档案名称',
  `device_name` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL COMMENT '默认的设备标示',
  `wifi_ssid` varchar(128) COLLATE utf8_unicode_ci NOT NULL COMMENT '默认的SSID号码',
  `wifi_encryption` varchar(128) COLLATE utf8_unicode_ci NOT NULL COMMENT '默认的加密方式',
  `wifi_key` varchar(128) COLLATE utf8_unicode_ci NOT NULL COMMENT '默认的密钥',
  `wifi_channel_mode` varchar(128) COLLATE utf8_unicode_ci NOT NULL COMMENT '默认的频道方式',
  `wifi_channel` varchar(24) COLLATE utf8_unicode_ci DEFAULT NULL COMMENT '手工的频道',
  `wifi_signal` varchar(16) COLLATE utf8_unicode_ci DEFAULT NULL COMMENT '默认的信号强度',
  PRIMARY KEY (`profile`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='默认的设置表，没有注册的话就会读取此处的设置';

-- --------------------------------------------------------

--
-- 表的结构 `node_setting`
--

CREATE TABLE IF NOT EXISTS `node_setting` (
  `pre_device_name` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL COMMENT '预设设备名',
  `pre_device_mac` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL COMMENT '预设设备MAC',
  `pre_device_description` text COLLATE utf8_unicode_ci COMMENT '预设设备描述',
  `device_latitude` varchar(20) COLLATE utf8_unicode_ci DEFAULT NULL,
  `device_longitude` varchar(20) COLLATE utf8_unicode_ci DEFAULT NULL,
  `wan_ip` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL,
  `wan_mac` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL,
  `wifi_ip` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL,
  `wifi_ssid` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL COMMENT '需要修改的wifi_ssid',
  `wifi_encryption` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL COMMENT '修改后的加密方式',
  `wifi_key` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL,
  `wifi_channel_mode` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL,
  `wifi_channel` varchar(24) COLLATE utf8_unicode_ci DEFAULT NULL COMMENT '修改后的频道',
  `wifi_signal` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL COMMENT '修改后的功率',
  UNIQUE KEY `pre_device_mac` (`pre_device_mac`),
  UNIQUE KEY `wan_mac` (`wan_mac`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='node database' AUTO_INCREMENT=2 ;

-- --------------------------------------------------------

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
