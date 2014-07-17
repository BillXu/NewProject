CREATE TABLE `log_shop` (
  `userUID` int(10) unsigned NOT NULL,
  `playerName` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `moneyType` int(10) unsigned zerofill DEFAULT NULL COMMENT '0 钻石， 1 金币，3  RMB',
  `spendMoney` int(10) unsigned zerofill DEFAULT NULL,
  `channel` smallint(5) unsigned zerofill DEFAULT NULL COMMENT '渠道标示，0. appstore  1. pp 助手，2.  91商店 3. 360商店 4.winphone store , 100 淘宝充值',
  `shopItemID` smallint(5) unsigned zerofill DEFAULT NULL,
  `shopItemCount` int(10) unsigned DEFAULT NULL,
  `coin` bigint(20) unsigned zerofill DEFAULT NULL,
  `diamoned` int(10) unsigned zerofill DEFAULT NULL,
  `logTime` int(10) unsigned DEFAULT NULL,
  PRIMARY KEY (`userUID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
