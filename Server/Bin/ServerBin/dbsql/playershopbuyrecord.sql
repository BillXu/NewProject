CREATE TABLE `playershopbuyrecord` (
  `nUserUID` int(10) unsigned NOT NULL,
  `pBuffer` blob,
  PRIMARY KEY (`nUserUID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
