DELIMITER $$
CREATE DEFINER=`root`@`127.0.0.1` PROCEDURE `CheckAccount`( IN strAccount varchar(200) , IN strPassword varchar(200) )
BEGIN
	declare nOutUID int unsigned default 0 ;
	declare nOutRet tinyint unsigned default 0 ; # 0 means ok , 1 account error , 2 passworld error, 3 state error ;
	declare nOutRegisterType tinyint unsigned default 0 ;
	declare strPass varchar(200) default null ;
	declare nState tinyint unsigned default 0 ;

	select userUID,password ,state , registerType from account where account = strAccount into nOutUID,strPass ,nState,nOutRegisterType ;
	if nOutUID = 0 or strPass = null then
		set nOutRet = 1 ;
	else
		if strPass = strPassword then
			if nState != 0 then 
				set nOutRet = 3 ;
				set nOutUID = 0 ;
			else 
				set nOutRet = 0 ;
			end if ;
		else
			set nOutRet = 2 ;
			set nOutUID = 0 ;
		end if ;
	end if ;
	select nOutRet,nOutUID,nOutRegisterType,strAccount;
END$$
DELIMITER ;
