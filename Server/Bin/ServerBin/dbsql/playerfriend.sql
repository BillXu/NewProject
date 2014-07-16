CREATE TABLE `playerfriend` (
  `UserUID` int(20) unsigned NOT NULL,
  `FriendUIDs` blob,
  PRIMARY KEY (`UserUID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='players'' friend info stored here ;';
