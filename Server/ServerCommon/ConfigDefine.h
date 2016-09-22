#pragma once
#define GAME_panda
#define GAME_paiYouQuan
#define GAME_365 

#if defined( GAME_panda)

#define Gotype_DevID "73d1ec14-84c9-46de-b6d7-fd9abbb4eb6d"
#define Gotype_ProdID "73d1ec14-84c9-46de-b6d7-fd9abbb4eb6d"
#define Wechat_notifyUrl ""
#define Wechat_MchID ""
#define Wechat_appID ""
#define Wechat_MchKey ""

#elif defined(GAME_paiYouQuan)

#define Gotype_DevID ""
#define Gotype_ProdID ""
#define Wechat_notifyPort ""
#define Wechat_MchID ""
#define Wechat_appID ""
#define Wechat_MchKey ""

#elif defined(GAME_365)

#define Gotype_DevID "8276e6d3-5e96-44e2-9243-97a40f019bb3"
#define Gotype_ProdID "8276e6d3-5e96-44e2-9243-97a40f019bb3"
#define Wechat_notifyUrl "http://abc.paiyouquan.com:5006/vxpay.php"
#define Wechat_MchID "1385365702"
#define Wechat_appID "wxae3a38cb9960bc84"
#define Wechat_MchKey "NUN5DKS5MJW4UBVJIL1G2XUQ66LU2ENU"

#else

#define Gotype_DevID ""
#define Gotype_ProdID ""
#define Wechat_notifyUrl "http://abc.paiyouquan.com:5006/vxpay.php"
#define Wechat_MchID ""
#define Wechat_appID ""
#define Wechat_MchKey ""

#endif 