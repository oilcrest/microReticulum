#pragma once

#include "Reticulum.h"
#include "Link.h"
#include "Identity.h"
#include "Bytes.h"
#include "Type.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <stdint.h>

namespace RNS {

	class Interface;
	class Packet;
	class Link;
	class Identity;

    /**
     * @brief A class used to describe endpoints in a Reticulum Network. Destination
	 * instances are used both to create outgoing and incoming endpoints. The
	 * destination type will decide if encryption, and what type, is used in
	 * communication with the endpoint. A destination can also announce its
	 * presence on the network, which will also distribute necessary keys for
	 * encrypted communication with it.
	 * 
	 * @param identity An instance of :ref:`RNS.Identity<api-identity>`. Can hold only public keys for an outgoing destination, or holding private keys for an ingoing.
	 * @param direction ``RNS.Destination.IN`` or ``RNS.Destination.OUT``.
	 * @param type ``RNS.Destination.SINGLE``, ``RNS.Destination.GROUP`` or ``RNS.Destination.PLAIN``.
	 * @param app_name A string specifying the app name.
	 * @param aspects Any non-zero number of string arguments.
	 */
	class Destination {

	public:
		class Callbacks {
		public:
			using link_established = void(*)(const Link &link);
			//using packet = void(*)(uint8_t *data, uint16_t data_len, Packet *packet);
			using packet = void(*)(const Bytes &data, const Packet &packet);
			using proof_requested = bool(*)(const Packet &packet);
		public:
			link_established _link_established = nullptr;
			packet _packet = nullptr;
			proof_requested _proof_requested = nullptr;
		friend class Detination;
		};

		//typedef std::pair<time_t, std::string> Response;
		using PathResponse = std::pair<time_t, Bytes>;
		//using PathResponse = std::pair<time_t, std::vector<uint8_t>>;

	public:
		Destination(Type::NoneConstructor none) {
			extreme("Destination NONE object created");
		}
		Destination(const Destination &destination) : _object(destination._object) {
			extreme("Destination object copy created");
		}
		Destination(const Identity &identity, const Type::Destination::directions direction, const Type::Destination::types type, const char* app_name, const char *aspects);
		virtual ~Destination() {
			extreme("Destination object destroyed");
		}

		inline Destination& operator = (const Destination &destination) {
			_object = destination._object;
			extreme("Destination object copy created by assignment, this: " + std::to_string((uintptr_t)this) + ", data: " + std::to_string((uintptr_t)_object.get()));
			return *this;
		}
		inline operator bool() const {
			return _object.get() != nullptr;
		}
		inline bool operator < (const Destination &destination) const {
			return _object.get() < destination._object.get();
		}

	public:
		static Bytes hash(const Identity &identity, const char *app_name, const char *aspects);
		static std::string expand_name(const Identity &identity, const char *app_name, const char *aspects);

	public:
		Packet announce(const Bytes &app_data = {}, bool path_response = false, Interface *attached_interface = nullptr, const Bytes &tag = {}, bool send = true);

		/*
		Set or query whether the destination accepts incoming link requests.

		:param accepts: If ``True`` or ``False``, this method sets whether the destination accepts incoming link requests. If not provided or ``None``, the method returns whether the destination currently accepts link requests.
		:returns: ``True`` or ``False`` depending on whether the destination accepts incoming link requests, if the *accepts* parameter is not provided or ``None``.
		*/
		inline void set_accepts_links(bool accepts) { assert(_object); _object->_accept_link_requests = accepts; }
		inline bool get_accepts_links() { assert(_object); return _object->_accept_link_requests; }

		/*
			Registers a function to be called when a link has been established to
			this destination.

			:param callback: A function or method with the signature *callback(link)* to be called when a new link is established with this destination.
		*/
		inline void set_link_established_callback(Callbacks::link_established callback) {
			assert(_object);
			_object->_callbacks._link_established = callback;
		}
		/*
			Registers a function to be called when a packet has been received by
			this destination.

			:param callback: A function or method with the signature *callback(data, packet)* to be called when this destination receives a packet.
		*/
		inline void set_packet_callback(Callbacks::packet callback)  {
			assert(_object);
			_object->_callbacks._packet = callback;
		}
		/*
			Registers a function to be called when a proof has been requested for
			a packet sent to this destination. Allows control over when and if
			proofs should be returned for received packets.

			:param callback: A function or method to with the signature *callback(packet)* be called when a packet that requests a proof is received. The callback must return one of True or False. If the callback returns True, a proof will be sent. If it returns False, a proof will not be sent.
		*/
		inline void set_proof_requested_callback(Callbacks::proof_requested callback)  {
			assert(_object);
			_object->_callbacks._proof_requested = callback;
		}

		/*
		Sets the destinations proof strategy.

		:param proof_strategy: One of ``RNS.Destination.PROVE_NONE``, ``RNS.Destination.PROVE_ALL`` or ``RNS.Destination.PROVE_APP``. If ``RNS.Destination.PROVE_APP`` is set, the `proof_requested_callback` will be called to determine whether a proof should be sent or not.
		*/
		inline void set_proof_strategy(Type::Destination::proof_strategies proof_strategy) {
			assert(_object);
			//if (proof_strategy <= PROOF_NONE) {
			//	throw throw std::invalid_argument("Unsupported proof strategy");
			//}
			_object->_proof_strategy = proof_strategy;
		}

		void receive(const Packet &packet);
		void incoming_link_request(const Bytes &data, const Packet &packet);

		Bytes encrypt(const Bytes &data);
		Bytes decrypt(const Bytes &data);
		Bytes sign(const Bytes &message);

		// getters/setters
		inline Type::Destination::types type() const { assert(_object); return _object->_type; }
		inline Type::Destination::directions direction() const { assert(_object); return _object->_direction; }
		inline Type::Destination::proof_strategies proof_strategy() const { assert(_object); return _object->_proof_strategy; }
		inline Bytes hash() const { assert(_object); return _object->_hash; }
		inline Bytes link_id() const { assert(_object); return _object->_link_id; }
		inline uint16_t mtu() const { assert(_object); return _object->_mtu; }
		inline void mtu(uint16_t mtu) { assert(_object); _object->_mtu = mtu; }
		inline Type::Link::status status() const { assert(_object); return _object->_status; }
		inline const Callbacks &callbacks() const { assert(_object); return _object->_callbacks; }
		inline const Identity &identity() const { assert(_object); return _object->_identity; }

		inline std::string toString() const { assert(_object); return "{Destination:" + _object->_hash.toHex() + "}"; }

	private:
		class Object {
		public:
			Object(const Identity &identity) : _identity(identity) {}
			virtual ~Object() {}
		private:
			bool _accept_link_requests = true;
			Callbacks _callbacks;
			//z_request_handlers = {}
			Type::Destination::types _type;
			Type::Destination::directions _direction;
			Type::Destination::proof_strategies _proof_strategy = Type::Destination::PROVE_NONE;
			uint16_t _mtu = 0;

			//std::vector<PathResponse> _path_responses;
			std::map<Bytes, PathResponse> _path_responses;
			//z_links = []

			Identity _identity;
			std::string _name;

			// Generate the destination address hash
			Bytes _hash;
			Bytes _name_hash;
			std::string _hexhash;

			// CBA TODO when is _default_app_data a "callable"?
			Bytes _default_app_data;
			//z_callback = None
			//z_proofcallback = None

			// CBA _link_id is expected by packet but only present in Link
			// CBA TODO determine if Link needs to inherit from Destination or vice-versa
			Bytes _link_id;

			Type::Link::status _status;

		friend class Destination;
		};
		std::shared_ptr<Object> _object;

	};

}