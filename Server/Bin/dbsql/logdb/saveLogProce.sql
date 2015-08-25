DELIMITER $$
CREATE DEFINER=`root`@`127.0.0.1` PROCEDURE `saveLog`(IN logType int unsigned ,  IN targetID int unsigned , IN externJson varchar(512), IN argid0 bigint , IN argid1 bigint , IN argid2 bigint, IN argid3 bigint, IN argid4 bigint, IN argid5 bigint )
BEGIN
	DECLARE now_datetime datetime DEFAULT '1999-12-31 23:59:59'; 
	set now_datetime = now();
	insert into log (logType,targetID,logTime,externJson,arg0,arg1,arg2,arg3,arg4,arg5) values(logType,targetID,now_datetime,externJson,argid0,argid1,argid2,argid3,argid4,argid5);
	select targetID ;
END$$
DELIMITER ;
