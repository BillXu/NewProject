CREATE TABLE `transnum` (
  `type` tinyint(4) NOT NULL,
  `transId` varchar(245) NOT NULL,
  PRIMARY KEY (`transId`),
  UNIQUE KEY `transId_UNIQUE` (`transId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='verify table ';
