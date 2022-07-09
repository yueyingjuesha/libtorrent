/*

Copyright (c) 2003, 2005, 2009, 2015-2017, 2020-2021, Arvid Norberg
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#include <cstdlib>
#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/session.hpp"
#include "libtorrent/torrent_info.hpp"

#include <iostream>

char const* esc(char const* code);
char const* timestamp()
{
	time_t t = std::time(nullptr);
	tm* timeinfo = std::localtime(&t);
	static char str[200];
	std::strftime(str, 200, "%b %d %X", timeinfo);
	return str;
}

void print_alert(lt::alert const* a)
{
	using namespace lt;
	std::printf("[%s] %s\n", timestamp(), a->message().c_str());
}

bool quit = false;

void signal_handler(int)
{
	// make the main loop terminate
	std::cout << "exiting main loop" << std::endl;
	quit = true;
}

int main(int argc, char* argv[]) try
{
	if (argc != 2) {
		std::cerr << "usage: ./simple_client torrent-file\n"
			"to stop the client, press return.\n";
		return 1;
	}

	/* auto settings = lt::high_performance_seed(); */
	auto settings = lt::default_settings();
	settings.set_int(lt::settings_pack::alert_mask,
		lt::alert_category::error

		| lt::alert_category::peer
		/* | lt::alert_category::port_mapping */
		| lt::alert_category::storage
		| lt::alert_category::tracker
		| lt::alert_category::connect
		| lt::alert_category::status
		| lt::alert_category::ip_block
		| lt::alert_category::performance_warning
		/* | lt::alert_category::dht */
		| lt::alert_category::session_log
		| lt::alert_category::torrent_log
		| lt::alert_category::peer_log
		| lt::alert_category::incoming_request
		| lt::alert_category::picker_log
		| lt::alert_category::piece_progress
		| lt::alert_category::file_progress
		| lt::alert_category::upload
		/* | lt::alert_category::dht_log */
		/* | lt::alert_category::dht_operation */
		/* | lt::alert_category::port_mapping_log */
		 );
	settings.set_str(lt::settings_pack::listen_interfaces, "0.0.0.0:25436,[::]:25436");
	settings.set_str(lt::settings_pack::user_agent, "Deluge/2.0.5 libtorrent/2.0.6.0");

	auto peer_fingerprint = lt::generate_fingerprint(std::string("DE"), 2, 0, 5, 0);
	settings.set_str(lt::settings_pack::peer_fingerprint, "-DE205s-");
	lt::session ses(lt::session_params(std::move(settings)));

	lt::add_torrent_params param;
	param.save_path = "/mnt/f/Download";
	param.ti = std::make_shared<lt::torrent_info>(argv[1]);
	const auto& torrent_handle = ses.add_torrent(param);
	/* FILE* log_file = std::fopen("/home/maxwillx/libtorrent/simple_client.log", "w+"); */

	std::vector<lt::alert*> alerts;
	signal(SIGTERM, signal_handler);
	signal(SIGINT, signal_handler);
	while (!quit) {

		ses.pop_alerts(&alerts);
		for (auto a : alerts) {
			print_alert(a);
		}

	}
	ses.remove_torrent(torrent_handle, lt::session::delete_files);
	std::cout << "torrent has been removed" << std::endl;
	return 0;
}
catch (std::exception const& e) {
	std::cerr << "ERROR: " << e.what() << "\n";
}

