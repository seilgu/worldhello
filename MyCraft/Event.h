
#include <map>


struct Event {
	static const int EV_NULL		= 0;
	static const int EV_PLAYER_LOOK = 1;
	static const int EV_PLAYER_MOVE = 2;
	unsigned long int game_time;
	unsigned int id;
	unsigned int type;
	int data;
};

struct event_compare {
	bool operator()(Event a, Event b) const {
		return (a.game_time < b.game_time);
	}
};

class EventMgr {
private:
	std::map<unsigned long int, Event, event_compare> m_events;

public:
	void PushEvent(Event evt);
	Event PopEvent();

	void ProcessEvents();
};