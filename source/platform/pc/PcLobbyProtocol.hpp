#ifndef PLATFORM_PC_PCLOBBYPROTOCOL_HPP
#define PLATFORM_PC_PCLOBBYPROTOCOL_HPP

const unsigned int kLobbyMagic = 0x424C434D;
const unsigned int kRelayMagic = 0x524C434D;
const unsigned int kLobbyHelloSize = 16;
const unsigned int kLobbyChallengeOffset = 16;
const unsigned int kLobbyChallengeSize = 16;
const unsigned short kLobbySignedVersion = 2;
const unsigned short kLobbyLinkingVersion = 3;
const unsigned char kLobbyShowsLinkCode = 2;
const unsigned char kLobbyKnowsFriends = 4;
const unsigned char kListedFriend = 1;
const unsigned char kListedFriendOfFriend = 2;
const unsigned char kVisibilityPublic = 0;
const unsigned char kVisibilityFriends = 1;
const unsigned char kVisibilityFriendsOfFriends = 2;
const char kIdentityContext[] = "LCE lobby identity v1";
const unsigned int kRelayFrameSize = 20;

const unsigned char kLobbyIdentify = 'I';
const unsigned char kLobbyWelcome = 'W';
const unsigned char kLobbyRefused = 'R';
const unsigned char kLobbyHosting = 'H';
const unsigned char kLobbyStopHosting = 'X';
const unsigned char kLobbyList = 'L';
const unsigned char kLobbyFriendList = 'F';
const unsigned char kLobbyVisibility = 'V';
const unsigned char kLobbyOpen = 'O';
const unsigned char kLobbyKeepAlive = 'P';
const unsigned char kLobbyLinkCode = 'D';
const unsigned char kRelayJoin = 'J';
const unsigned char kRelayAccept = 'A';
const unsigned char kRelayReady = 'K';

const unsigned short kLobbyPort = 25565;
const unsigned short kLobbyAlternatePort = 25570;

#endif
