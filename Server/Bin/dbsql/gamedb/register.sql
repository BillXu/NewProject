DELIMITER $$
CREATE DEFINER=`root`@`127.0.0.1` PROCEDURE `RegisterAccount`(IN strAccount varchar(255) , IN strPassword varchar(255) , IN nRegisterType tinyint unsigned , IN nChangel tinyint unsigned )
BEGIN
	DECLARE now_datetime datetime DEFAULT '1999-12-31 23:59:59'; 
	DECLARE nOutRet tinyint unsigned default 1 ;
	DECLARE nOutUserUID int unsigned default 0 ;
	DECLARE nCheckUserUID int unsigned default 0 ;
	#if strPassword = null then 
	#	set strPassword = '';
	#end if ;
	#declare nEffectRow int unsigned default 0; 
	select userUID from  account where account = strAccount into nCheckUserUID ;
	if nCheckUserUID = 0 then
	    set nOutRet = 0 ;
		set now_datetime = now();
		insert into account (account,password,registerType,channel,registerDateTime) values(strAccount,strPassword,nRegisterType,nChangel,now_datetime);
		#select userUID from  account where account = strAccount into nOutUserUID ;
		set nOutUserUID = LAST_INSERT_ID();
	end if  ;

	select nOutRet,nOutUserUID,strAccount,strPassword;
END$$
DELIMITER ;
