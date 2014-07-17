CREATE TABLE `playeritems` (
  `userUID` int(10) unsigned NOT NULL,
  `ownItemsKindCount` smallint(5) unsigned zerofill DEFAULT '00000',
  `itemsData` blob,
  PRIMARY KEY (`userUID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
