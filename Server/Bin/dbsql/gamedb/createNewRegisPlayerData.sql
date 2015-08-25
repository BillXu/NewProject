DELIMITER $$
CREATE DEFINER=`root`@`127.0.0.1` PROCEDURE `CreateNewRegisterPlayerData`( IN nNewUserUID int unsigned, IN strName varchar(200) )
BEGIN
	declare nOutRet tinyint unsigned default 0 ;
	declare nRowCnt tinyint unsigned default 0 ;
	declare nInsertCnt tinyint unsigned default 0 ;
	
	# inte playerbasedata 
	insert into playerbasedata (userUID,playerName) values(nNewUserUID,strName);
	set nRowCnt = nRowCnt + ROW_COUNT();
	set nInsertCnt = nInsertCnt + 1;
	
	#just erver insert ok 
	if nRowCnt != nInsertCnt then
		set nOutRet = 1 ;
	end if ;
	select nOutRet , nNewUserUID;
END$$
DELIMITER ;
