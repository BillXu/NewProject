DELIMITER $$
CREATE DEFINER=`root`@`127.0.0.1` PROCEDURE `ModifyPassword`( IN nCurUserUID int unsigned , IN strOldPassword varchar(200) , IN strNewPassword varchar(200) )
BEGIN
	declare nOutRet tinyint unsigned default 0 ; # 0 success , 1 uid not exsit , 2 old passworld error 
	declare strselPass varchar(200) default null ;
	if strNewPassword != strOldPassword then
		select password from account where userUID = nCurUserUID into strselPass ;
		if isnull(strselPass) then 
			set nOutRet = 1 ;
		elseif strselPass = strOldPassword then
			update account set password = strNewPassword where userUID = nCurUserUID ; 
		else 
			set nOutRet = 2 ;
		end if ;
	end if ;
	select nOutRet ;
END$$
DELIMITER ;
