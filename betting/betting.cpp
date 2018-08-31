// vim: tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab

#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/time.hpp>
#include <eosiolib/asset.hpp>
#include <ctime>
#include <eosiolib/transaction.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/currency.hpp>
#include <string.h>

using namespace eosio;
using namespace std;

using eosio::asset;


/*
struct betting
{
    uint64_t 	id;
    account_name to;
    asset	bet;
    uint64_t 	gameid = 0;
    uint32_t 	betting_time = 0;

    uint64_t	primary_key() const {return id;}
    EOSLIB_SERIALIZE(betting,(id)(to)(bet)(gameid) )
};
typedef multi_index<N(betting),betting> _ebetting;
*/

class hello : public eosio::contract {
  public:
      using contract::contract;
	/// @abi table 
	struct tgame
	{
	    uint64_t  	game_id = 0;
	    uint64_t  	start_time = 0;
	    uint64_t  	end_time = 0;
	    asset	bet;
	    uint64_t	rnd = 0;
	    uint64_t 	primary_key() const {return game_id;}
	    EOSLIB_SERIALIZE( tgame, (game_id)(start_time)(end_time)(bet)(rnd) )

	};
	typedef  eosio::multi_index<N(tgame),tgame> game_index;

      /// @abi action 
      void hi( account_name user ) {
	curtime();
         print( "Hello, ", name{user} );
      }

	void gameremove(uint64_t game_id)
	{
		require_auth(_self);
		game_index games(_self,_self);
		auto iter=games.find(1);
		games.erase(iter);
	}


	void gameregist(uint64_t start_time, uint64_t end_time)
	{
		require_auth(_self);
		game_index games(_self,_self);
		auto iter=games.find(1);
           	if(iter==games.end())
		{
			// 종료시간이 현재 시간보다 작으면 오류내도록 
			if ( end_time < curtime() )
			{
				print("end_time less then current time : current time => " , curtime() , "\t");
			} else {
				//종료 시간이 현재시간보다 클 경우만 게임인서트 할수 있도록 
				_regist_new_game(start_time, end_time);
				print("new game insert completed\t");
			}
		} else if ( iter->start_time > curtime() ) {
			// 등록된 게임의 시작시간이 현재시간 보다 작을 경우
			// 아직 등록중인 게임이 있습니다. 
			print_f("ready new game, %l will started \t", iter->start_time);
		} else if ( iter->end_time < curtime() ) {
			// 아직종료되지 않은 게임이 있어서 신규 게임을 등록 할 수 없음
			print_f("do not finished yet(%l will finished) \t", iter->end_time);
		} else {
			// 기존 게임 삭제 후 신규게임 등록 
			gameremove(1);
			_regist_new_game(start_time, end_time);
			print("new game insert completed\t");
		}
	} 

	void _regist_new_game(uint64_t start_time, uint64_t end_time)
	{
		uint64_t rnd = _getrand();
		print_f("insert value : (start time % , end time %,rnd % )\t", start_time, end_time , rnd);
		game_index games(_self,_self);
		games.emplace(_self , [&] ( auto& new_game) 
		{
			new_game.game_id = 1;
			new_game.start_time = start_time;
			new_game.end_time = end_time;
			new_game.rnd = rnd;
		});
		gamelist();
	}

	void nowtime()
	{
		print ("current unixtime : %",curtime());

	}
	void addtime(uint64_t addsec)
	{
		uint64_t tmp = 0;
		tmp = curtime();
		print ("current unixtime : ",tmp,",after unixtime :",(tmp+addsec));
	}

	uint64_t curtime() {
		uint64_t tmp = 0;
		tmp = current_time();
		return tmp/(1000*1000);
	}

	int64_t	_getrand()
	{
		return (curtime()%100);
	}
	
	void gamelist()
	{
		game_index games(_self,_self);
		auto iter=games.find(1);
	//	print_f("registed game info : (start_time %, end_time %, rnd %)\t", iter->start_time, iter->end_time, iter->rnd);

	}	

	uint64_t getGameRand()
	{
		game_index games(_self,_self);
		auto iter=games.find(1);
		return iter->rnd;
	}

	asset	getBettingToken()
	{
		game_index games(_self,_self);
		auto iter=games.find(1);
		return iter->bet;
	}


	/// @abi action
	void distribute( uint64_t receiver, uint64_t code ) {
		// EOS 토큰이 아닌경우, 이벤트 처리 없이 종료
		//if(code != N(eosio.token)) return;
		eosio_assert(code == N(eosio.token), "I reject your non-eosio.token deposit");

		auto data = unpack_action_data<currency::transfer>();
		if(data.from == _self || data.to != _self) return;

		 eosio_assert(data.quantity.symbol == string_to_symbol(4, "EOS"), "EOS 타입이 아닙니다.");
		 eosio_assert(data.quantity.is_valid(), "SYMBOL 타입이 맞지 않습니다.");
		 eosio_assert(data.quantity.amount > 0, " 보낼 수량이 부족합니다.");


	}

	//@abi action
	void exchange( uint64_t receiver, uint64_t code )
	{
		uint64_t amount = 0;
		string sFromAccount = "";
		string sFromAccountTest = "eos2okayjava";
		// EOS토큰이 아닌경우, 이벤트 처리 없이 종료
		if(code != N(eosio.token)) return;

		// #아래 조건문은 EOS가 아닌경우, 트랜잭션을 리젝 시키기 때문에, 다른 토큰을 받을 수가 없음 
		// eosio_assert(code == N(eosio.token), "I reject your non-eosio.token deposit");

		auto data = unpack_action_data<currency::transfer>();
		if(data.from == _self || data.to != _self) return;

		amount =  data.quantity.amount/10000;

		if ( getGameRand() == amount )
		{
			print_f(",hint : exjectly\t");
		} else if ( getGameRand() > amount ) {
			print_f(",hint : down\t");
		} else if ( getGameRand() < amount ) {
			print_f(",hint : up\t");
		}

		// 배팅금액 업데이트
	        asset tokens;
		tokens.symbol = string_to_symbol(4, "EOS");
		tokens.amount = data.quantity.amount;
		game_index games(_self,_self);
		games.modify( games.begin(), 0 , [&](auto & ii) {
			ii.bet += tokens;
		});
		print_f(",total Betting: %\t", getBettingToken());


	}


};

#define EOSIO_ABI2( TYPE, MEMBERS ) \
extern "C" { \
   void apply( uint64_t receiver, uint64_t code, uint64_t action ) { \
      auto self = receiver; \
      if( action == N(onerror)) { \
         /* onerror is only valid if it is for the "eosio" code account and authorized by "eosio"'s "active permission */ \
         eosio_assert(code == N(eosio), "onerror action's are only valid from the \"eosio\" system account"); \
      } \
      if( code == self || action == N(onerror) ) { \
         TYPE thiscontract( self ); \
         switch( action ) { \
            EOSIO_API( TYPE, MEMBERS ) \
         } \
         /* does not allow destructor of thiscontract to run: eosio_exit(0); */ \
      } \
      else { \
         TYPE thiscontract( self ); \
         switch( action ) { \
            case N(transfer): return thiscontract.exchange(receiver, code); \
         } \
         /* does not allow destructor of thiscontract to run: eosio_exit(0); */ \
      } \
   } \
} \

EOSIO_ABI2( hello, (hi)(addtime)(nowtime)(gameregist)(gameremove)(gamelist)(distribute)(exchange) )
