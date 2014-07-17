CREATE TABLE `masterstudent` (
  `userUID` int(10) unsigned NOT NULL,
  `masterAndStudentCount` smallint(5) unsigned zerofill DEFAULT '00000',
  `contentData` blob,
  PRIMARY KEY (`userUID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
