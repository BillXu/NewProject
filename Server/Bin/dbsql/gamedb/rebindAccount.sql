DELIMITER $$
CREATE DEFINER=`root`@`127.0.0.1` PROCEDURE `RebindAccount`( IN nUserUID int unsigned , IN strAccount varchar(200), IN strPassword varchar(200) )
BEGIN
	declare nOutRet tinyint unsigned default 0 ;  # 0 success , 1 strAccount dublicate , 2 nUserUID not exsit 
	declare nSelUID int unsigned default null ;

	select userUID from account where account = strAccount into nSelUID;
	if isnull(nSelUID) then
		update account set account = strAccount , password = strPassword, registerType = 2  where userUID = nUserUID ; 
		if ROW_COUNT() = 0 then 
			set nOutRet = 2 ;
		end if ;
	else 
		set nOutRet = 1 ;
	end if ;
	select nOutRet ;
END$$
DELIMITER ;
