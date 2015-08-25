DELIMITER $$
CREATE DEFINER=`root`@`127.0.0.1` PROCEDURE `addCoinByUID`(IN nCoinToAdd int unsigned, IN nUserUID int unsigned )
BEGIN
declare nPlayerCoin bigint default 0 ;
select coin from playerbasedata where userUID = nUserUID into nPlayerCoin ;
#select coin from playerbasedata where userUID = nUserUID ;
SET nPlayerCoin = nPlayerCoin + nCoinToAdd;
update playerbasedata set coin = nPlayerCoin where userUID = nUserUID ;
#select coin from playerbasedata where userUID = nUserUID ;
END$$
DELIMITER ;
