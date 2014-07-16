CREATE TABLE `playeritems` (
  `nUserUID` int(11) NOT NULL,
  `Items` blob,
  PRIMARY KEY (`nUserUID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='all player items ';
