#ifndef _ZEGAR_H
#define _ZEGAR_H

//kwarc 32,768 Hz
#define COMPARE 2 // 64 prescaler * 256 counter * {2}  = 32,768 (przepe³nienie z chêstotliwoœci¹ f = 2Hz)


enum mode { H_M, M_S };


class Zegar{
public:
	Zegar();
	Zegar(uint8_t h, uint8_t min, uint8_t sec = 0);
	void static init();

	void start();
	void stop();
	void reset();
	void work();

	void increment_seconds();
	void increment_minutes();
	void increment_hours();

	void set_display(bool value);
	void on_pulse(); // wykonywane w trakcie przerwania timera
	void set_mode(mode m);

private:
	uint8_t j,d,s,t;
	uint8_t secs, mins, hours;
	uint8_t core; // wewnêtrzny licznik zegara
	uint8_t position; // pozycja cyfry na wyœwietlaczu
	mode m; // tryb wyœwietlania
	bool ticking; // czy zegar odmierza czas
	bool display; // czy wyœwietlac godzinê
	bool pulse; // impuls z g³ównego generatora (zwiêksza licznik zegara 'core')

	void tick();
	void show(bool dot_on);

	void take_hours_mins();
	void take_mins_secs();
	void blink_dot();

	void refresh_digits();

};

#endif
