DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `RegisterRobot`(IN nStartIdx int unsigned , IN nEndIdx int unsigned, IN sRobotPre varchar(200), IN nBeginUID int unsigned )
BEGIN
declare sRobotName varchar(500) ;
while nStartIdx <= nEndIdx do
set sRobotName = CONCAT(sRobotPre ,nStartIdx) ;
insert into account (account,password,userUID) values(sRobotName,"1",nBeginUID);
INSERT INTO playerbasedata (userUID, defaultPhotoID, sex, playerName) VALUES (nBeginUID, 0, 0, sRobotName);  
INSERT INTO masterstudent (userUID) VALUES (nBeginUID); 
INSERT INTO playerfriend (userUID) VALUES (nBeginUID);  
INSERT INTO playeritems (userUID) VALUES (nBeginUID);  
INSERT INTO playermission (userUID) VALUES (nBeginUID);  
set nBeginUID = nBeginUID + 1 ;
set nStartIdx = nStartIdx + 1 ;
end while;
END$$
DELIMITER ;
