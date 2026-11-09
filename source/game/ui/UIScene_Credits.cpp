#include "game/ui/UIScene_Credits.hpp"
#include "game/core/StringIds.hpp"

#include <cstring>
#include <cwchar>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DLCLocalisationFile.hpp"
#include "game/core/DLCManager.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/L10N.hpp"
#include "game/core/StringUtils.hpp"
#include "game/data/StringTable.hpp"
#include "game/ui/FJ_CreditsMenu.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/XLanguage.hpp"
#include "game/client/render/Renderer.hpp"

extern ConsoleUIController gConsoleUIController;

namespace
{
const int CREDIT_IMAGE = -2;
const int PRELOADED_LABEL_LENGTH = 128;
const int LABEL_LENGTH = 512;
const int LABEL_BUFFER_LENGTH = 256;

SCreditTextItemDef CREDIT_TEXT[] = {
    {L"MOJANG", IDS_NONE, IDS_NONE, ECreditTextTypes_Title},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_MINECRAFT_TEAM, IDS_NONE, ECreditTextTypes_Role},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_ORIGINAL_CREATOR, IDS_NONE, ECreditTextTypes_Role},
    {L"Markus Persson", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_LEAD_CREATIVE_DESIGNER, IDS_NONE, ECreditTextTypes_Role},
    {L"Jens Bergensten", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_PRODUCTION_DIRECTOR, IDS_NONE, ECreditTextTypes_Role},
    {L"Daniel Kaplan", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_GAME_DEVELOPERS, IDS_NONE, ECreditTextTypes_Role},
    {L"Agnes Larsson", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_ALEXANDER_STMAN, IDS_NONE, ECreditTextTypes_Text},
    {L"Aron Nieminen", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Daniel Wustenhoff", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Erik Broes", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Erik Bylund", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Johan Bernhardsson", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Jon Maiga", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_MARIA_LEMN, IDS_NONE, ECreditTextTypes_Text},
    {L"Michael Stoyke", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Mikael Hedberg", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_MNS_OLSON, IDS_NONE, ECreditTextTypes_Text},
    {L"Nathan Adams", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Ryan Holtz", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Sebastian Hindefelt", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Shoghi Cervantes", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_THOMAS_GUIMBRETIRE, IDS_NONE, ECreditTextTypes_Text},
    {L"Tomas Alaeus", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Tommaso Checchi", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_REALMS_DEVELOPERS, IDS_NONE, ECreditTextTypes_Role},
    {L"Alexandre Pretto Nunes", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Amir Moulavi", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Christian Westman", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Daniel Frisk", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"David Marby", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Leonard Gram", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_MAGNUS_JDERBERG, IDS_NONE, ECreditTextTypes_Text},
    {L"Mats Henricson", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Philip Vieira", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_ADDITIONAL_PROGRAMMING, IDS_NONE, ECreditTextTypes_Role},
    {L"Paul Spooner", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Ryan 'Scaevolus' Hitchman", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Elliot 'Hippoplatimus' Segal", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_ASSISTANT_PRODUCERS, IDS_NONE, ECreditTextTypes_Role},
    {L"%ls", IDS_CREDITS_ADRIAN_STERGRD, IDS_NONE, ECreditTextTypes_Text},
    {L"Aleksandra Zajac", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_REALMS_CONTENT_LEAD, IDS_NONE, ECreditTextTypes_Role},
    {L"Marc Watson", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_REALMS_CONTENT_COORDINATOR, IDS_NONE, ECreditTextTypes_Role},
    {L"Matthew Dryden", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_ART_DIRECTOR, IDS_NONE, ECreditTextTypes_Role},
    {L"Markus Toivonen", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_GRAPHICS_ARTISTS, IDS_NONE, ECreditTextTypes_Role},
    {L"Johan Aronsson", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Kristoffer Zetterstrand", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_ADDITIONAL_GRAPHICS, IDS_NONE, ECreditTextTypes_Role},
    {L"%ls", IDS_CREDITS_JONATAN_PLJ, IDS_NONE, ECreditTextTypes_Text},
    {L"Mariana Graham Ramirez", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Martin Johansson", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Ninni Landin", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_USER_EXPERIENCE_DESIGN, IDS_NONE, ECreditTextTypes_Role},
    {L"Tobias Ahlin", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_LEAD_SOUND_DESIGNER, IDS_NONE, ECreditTextTypes_Role},
    {L"%ls", IDS_CREDITS_SAMUEL_BERG, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_SOUND_DESIGN, IDS_NONE, ECreditTextTypes_Role},
    {L"Daniel Rosenfeld", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_MUSIC_COMPOSED_BY, IDS_NONE, ECreditTextTypes_Role},
    {L"Daniel Rosenfeld", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_WRITING, IDS_NONE, ECreditTextTypes_Role},
    {L"Julian Gough", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_DEVELOPERS_OF_MO_CREATURES, IDS_NONE, ECreditTextTypes_Role},
    {L"John Olarte (DrZhark)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Kent Christian Jensen", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Dan Roque", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Mojang", IDS_NONE, IDS_NONE, ECreditTextTypes_Title},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_CHIEF_EXECUTIVE_OFFICER, IDS_NONE, ECreditTextTypes_Role},
    {L"%ls", IDS_CREDITS_JONAS_MRTENSSON, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_CHIEF_OPERATIONS_OFFICER, IDS_NONE, ECreditTextTypes_Role},
    {L"Vu Bui", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_CHIEF_FINANCE_OFFICER, IDS_NONE, ECreditTextTypes_Role},
    {L"Karin Severinson", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_BRAND_DIRECTOR, IDS_NONE, ECreditTextTypes_Role},
    {L"Lydia Winters", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_CHIEF_TECHNOLOGY_OFFICER, IDS_NONE, ECreditTextTypes_Role},
    {L"Rikard Herlitz", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_LEAD_SYSTEMS_DEVELOPER, IDS_NONE, ECreditTextTypes_Role},
    {L"Kristoffer Jelbring", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_SYSTEM_DEVELOPERS, IDS_NONE, ECreditTextTypes_Role},
    {L"%ls", IDS_CREDITS_PR_AXELSSON, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_ROBERT_SJDAHL, IDS_NONE, ECreditTextTypes_Text},
    {L"Stefan Torstensson", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Tomas Sommar", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_CUSTOMER_SUPPORT_TEAM_LEAD, IDS_NONE, ECreditTextTypes_Role},
    {L"Mattias Victorin", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_CUSTOMER_SUPPORT, IDS_NONE, ECreditTextTypes_Role},
    {L"Adam Martinsson", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Amelia Lindroth Henriksson", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Angehlica Walling", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Anna Lundgren", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Antonia Kousathana", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Carl Johnsson", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Cim Borg", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Eliza Hearsum", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Ellie Ashrafi", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Jonny Hair", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Marcus Forss", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Nasim Derakhshan", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_PATRIK_SDERGREN, IDS_NONE, ECreditTextTypes_Text},
    {L"Rui Ribeiro", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_SARAH_MRTENSSON, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_DIRECTOR_OF_BUSINESS_DEVELOPMENT, IDS_NONE, ECreditTextTypes_Role},
    {L"Patrick Geuder", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_BUSINESS_INTELLIGENCE, IDS_NONE, ECreditTextTypes_Role},
    {L"Brynjolfur Erlingsson", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_PRODUCTION_DIRECTOR, IDS_NONE, ECreditTextTypes_Role},
    {L"Olof Carlsson", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_PRODUCERS, IDS_NONE, ECreditTextTypes_Role},
    {L"Hampus Nilsson", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_PROJECT_DIRECTOR, IDS_NONE, ECreditTextTypes_Role},
    {L"%ls", IDS_CREDITS_ULRIKA_HJGRD, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_PROJECT_MANAGERS, IDS_NONE, ECreditTextTypes_Role},
    {L"Henrik Pettersson", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Isabella Balk", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Lisa Kempe", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Nicolette Suraga", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_DIRECTOR_OF_CREATIVE_COMMUNICATIONS, IDS_NONE, ECreditTextTypes_Role},
    {L"Owen Jones", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_CREATIVE_COMMUNICATIONS_MANAGER, IDS_NONE, ECreditTextTypes_Role},
    {L"Martin Davies", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_CREATIVE_COMMUNICATIONS_ASSISTANT, IDS_NONE, ECreditTextTypes_Role},
    {L"Tom Stone", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_HEAD_OF_COMMUNITY_RELATIONS, IDS_NONE, ECreditTextTypes_Role},
    {L"%ls", IDS_CREDITS_KARIM_WALLDN, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_HEAD_OF_SOCIAL_MEDIA, IDS_NONE, ECreditTextTypes_Role},
    {L"Amelia Dale", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_OFFICE_MANAGERS, IDS_NONE, ECreditTextTypes_Role},
    {L"David Stuart Dahlgren", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Mikaela Prim", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Siri Hoel Andersson", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_FINANCE_MANAGER, IDS_NONE, ECreditTextTypes_Role},
    {L"Marina Kostesic", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_FINANCE_ACCOUNTANTS, IDS_NONE, ECreditTextTypes_Role},
    {L"Josefina Axelsson", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_GRAPHICS_ARTISTS, IDS_NONE, ECreditTextTypes_Role},
    {L"Mattis Grahm", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", 0xD7F366C1, IDS_NONE, ECreditTextTypes_Role},
    {L"Christian Nordgren", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_LEVEL_DESIGNERS, IDS_NONE, ECreditTextTypes_Role},
    {L"Max Herngren", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_PRODUCT_DESIGNERS, IDS_NONE, ECreditTextTypes_Role},
    {L"%ls", IDS_CREDITS_AMANDA_STRM, IDS_NONE, ECreditTextTypes_Text},
    {L"Filip Thoms", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_INTELLECTUAL_PROPERTY_ENFORCEMENT_TEAM_LEAD, IDS_NONE, ECreditTextTypes_Role},
    {L"Mathias Andersson", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_INTELLECTUAL_PROPERTY_ENFORCEMENT_AGENTS, IDS_NONE, ECreditTextTypes_Role},
    {L"Josefin Olsson", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_MATILDA_KERMAN, IDS_NONE, ECreditTextTypes_Text},
    {L"Olle Personne", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Mojang Alumni", IDS_NONE, IDS_NONE, ECreditTextTypes_Title},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_CHIEF_EXECUTIVE_OFFICERS, IDS_NONE, ECreditTextTypes_Role},
    {L"Carl Manneh", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_PROJECT_DIRECTORS, IDS_NONE, ECreditTextTypes_Role},
    {L"Jakob Porser", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_GAME_DEVELOPERS, IDS_NONE, ECreditTextTypes_Role},
    {L"%ls", IDS_CREDITS_TOBIAS_MLLSTAM, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_SYSTEM_DEVELOPERS, IDS_NONE, ECreditTextTypes_Role},
    {L"Martin Odhelius", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_GRAPHICS_ARTISTS, IDS_NONE, ECreditTextTypes_Role},
    {L"Poi Poi Chen", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_CUSTOMER_SUPPORT, IDS_NONE, ECreditTextTypes_Role},
    {L"Andreas Andersson", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_ANDREAS_JRGENSEN, IDS_NONE, ECreditTextTypes_Text},
    {L"Annika Tripke-Lund", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Anton Albiin", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Dante Stjernberg", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"David Carlsson", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Elin Frykholm", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Emily Richardson", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Erik Nordberg", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Fredrik Henriksson", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Henrik Davallius", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Henry Shi", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Joe Liu", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Kevin Vesterlund", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Mike Till", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Rabi Hadad", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Robert Miskiewicz", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Robin Cocks", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_ROBIN_THUNSTRM, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_OFFICE_MANAGERS, IDS_NONE, ECreditTextTypes_Role},
    {L"%ls", IDS_CREDITS_CHARLOTTE_WREDESJ, IDS_NONE, ECreditTextTypes_Text},
    {L"Jill Curran", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Linn Hultman", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_SPECIAL_THANKS, IDS_NONE, ECreditTextTypes_Title},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Daniel Brynolf", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Pontus Hammarberg", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Title},
    {L"4J Studios", IDS_NONE, IDS_NONE, ECreditTextTypes_Title},
    {L"%ls", IDS_CREDITS_PROGRAMMING, IDS_NONE, ECreditTextTypes_Role},
    {L"Paddy Burns", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Richard Reavy", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Stuart Ross", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Mark Hughes", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Thomas Kronberg", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Vince Finlayson", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Iain Everett", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"David Shindler", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Calum Anderson", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"James Vaughan", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"George Vaughan", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Harry Gordon", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_ART, IDS_NONE, ECreditTextTypes_Role},
    {L"David Keningale", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Kate Wright", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Michael Hansen", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Thomas Naylor", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Alan Redmond", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Donald Robertson", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Chris Reeves", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Brian Lindsay", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Julie Kennedy", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Jonathan Vigurs", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Sidney Flavell", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Scott Jarron", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Rebecca O'Neil", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Jess Hider", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Jamie Keddie", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Hannah Watts", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Magnus Stone", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Chloe Smyth", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Natalie Clayton", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Cari Watterton", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Emma Rydstrom", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Ryan Mitchell", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Oliver Dowling", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Thomas Fyfe", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Bev Campbell", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Josef Boon", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Constantin Popa", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_QA, IDS_NONE, ECreditTextTypes_Role},
    {L"Steven Gary Woodward", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Rory Gatens", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Tom Mullender", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Oliver Gilchrist", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_SPECIALTHANKS, IDS_NONE, ECreditTextTypes_Role},
    {L"Chris van der Kuyl", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Roni Percy", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Anne Clarke", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Anthony Kent", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Microsoft Studios", IDS_NONE, IDS_NONE, ECreditTextTypes_Title},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Minecraft Console Team", IDS_NONE, IDS_NONE, ECreditTextTypes_Role},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_EXECUTIVE_PRODUCER, IDS_NONE, ECreditTextTypes_Role},
    {L"Roger Carpenter", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_SENIOR_PRODUCER, IDS_NONE, ECreditTextTypes_Role},
    {L"Yutaka Noma", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Riccardo Lenzi", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_PRODUCER, IDS_NONE, ECreditTextTypes_Role},
    {L"Micky Yamaguchi", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Damian Finn (OPM Response)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_QUALITY_TEAM_LEAD, IDS_NONE, ECreditTextTypes_Role},
    {L"Rich Levy", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_MINECRAFT_TEST_TEAM_LEAD, IDS_NONE, ECreditTextTypes_Role},
    {L"Chris Henry", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_STUDIOS_RELEASE_MANAGEMENT, IDS_NONE, ECreditTextTypes_Role},
    {L"Andrew Pritchard (OPM Response)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Nilesh Thakkar", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Josh Mulanax", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Jim Pekola (Insight Global)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Jordan Forbes (Insight Global)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Joshua Bullard (Insight Global)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_PRODUCTION_DIRECTOR, IDS_NONE, ECreditTextTypes_Role},
    {L"Travis Howland", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_BUSINESS_DEVELOPMENT, IDS_NONE, ECreditTextTypes_Role},
    {L"Cherie Lutz", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Peter Zetterberg", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_FRANCHISE_BUSINESS_MANAGER, IDS_NONE, ECreditTextTypes_Role},
    {L"Josh Bliggenstorfer", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_FRANCHISE_BUSINESS_TEAM, IDS_NONE, ECreditTextTypes_Role},
    {L"Nick Edwards", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Aaron Buckley", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Eliot Lee (Xtreme Staffing)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_MARKETING, IDS_NONE, ECreditTextTypes_Role},
    {L"Jessica Freeman", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Jeff Rivait", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Matt Percy", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_JAIME_LIMN, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_SENIOR_MANAGER_PLAYER_VOICE_EXPERIENCE, IDS_NONE, ECreditTextTypes_Role},
    {L"Aubrey Norris", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_PUBLIC_RELATIONS, IDS_NONE, ECreditTextTypes_Role},
    {L"Regan O'Leary", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_COMMUNITY_MANAGER, IDS_NONE, ECreditTextTypes_Role},
    {L"Helen Zbihlyj", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_COMMUNITY_COORDINATOR, IDS_NONE, ECreditTextTypes_Role},
    {L"Matt Gartzke (Experis) ", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Jay Wells", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Jess Robinson", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Nova Barlow", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_LOCALIZATION, IDS_NONE, ECreditTextTypes_Role},
    {L"%ls", IDS_CREDITS_LOCALIZATION_ENGINEERING_MANAGER, IDS_NONE, ECreditTextTypes_Role},
    {L"Chad Dylan Long", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_LOCALIZATION_PROJECT_MANAGER, IDS_NONE, ECreditTextTypes_Role},
    {L"Justin Hartsfield (Experis)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_LOCALIZATION_TEST_PROJECT_MANAGER, IDS_NONE, ECreditTextTypes_Role},
    {L"Konrad Lesiak (Lionbridge Game Services)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_SENIOR_LOCALIZATION_TEST_LEAD, IDS_NONE, ECreditTextTypes_Role},
    {L"Steffen Cornelius Strohmann (Lionbridge Game Services)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_LOCALIZATION_TEST_LEAD, IDS_NONE, ECreditTextTypes_Role},
    {L"Krzysztof Czyz (Lionbridge Game Services)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_GAMES_TEST_SPECIALISTS_FRENCH, IDS_NONE, ECreditTextTypes_Role},
    {L"Sofiane Draa (Lionbridge Game Services)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Pablo Bellostas (Lionbridge Game Services)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Alexander Herrera Alonso (Lionbridge Game Services)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Jeremie Dion (Lionbridge Game Services)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_GAMES_TEST_SPECIALISTS_ITALIAN, IDS_NONE, ECreditTextTypes_Role},
    {L"Marco Salerno (Lionbridge Game Services)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Michele De Capua (Lionbridge Game Services)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_GAMES_TEST_SPECIALISTS_GERMAN, IDS_NONE, ECreditTextTypes_Role},
    {L"Laura Strohmann (Lionbridge Game Services)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Oliver Urban (Lionbridge Game Services)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Eryk Schleiermacher (Lionbridge Game Services)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_GAMES_TEST_SPECIALISTS_SPANISH, IDS_NONE, ECreditTextTypes_Role},
    {L"Diego Fernandez Rodriguez (Lionbridge Game Services)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Francisco Javier Aguilar Jurado (Lionbridge Game Services)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_GAMES_TEST_SPECIALISTS_PORTUGUESE, IDS_NONE, ECreditTextTypes_Role},
    {L"Joao Meira (Lionbridge Game Services)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Martim Graca Dias (Lionbridge Game Services)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_GAMES_TEST_SPECIALISTS_RUSSIAN, IDS_NONE, ECreditTextTypes_Role},
    {L"Eugenia Iwlewa (Lionbridge Game Services)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Vitali Kunashka (Lionbridge Game Services)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Games Test Specialists (Dutch)", IDS_NONE, IDS_NONE, ECreditTextTypes_Role},
    {L"Erwin Schaars (Lionbridge Game Services)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Hugo Los (Lionbridge Game Services)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_GAMES_TEST_SPECIALISTS_TRADITIONAL_CHINESE, IDS_NONE, ECreditTextTypes_Role},
    {L"Si-Cai Lin (Lionbridge Game Services)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Sheng Feng Lin (Lionbridge Game Services)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Wen-Yu Chen (Lionbridge Game Services)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_GAMES_TEST_SPECIALISTS_SIMPLIFIED_CHINESE, IDS_NONE, ECreditTextTypes_Role},
    {L"WeiJia Gu (Lionbridge Game Services)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_GAMES_TEST_SPECIALISTS_JAPANESE, IDS_NONE, ECreditTextTypes_Role},
    {L"Mei Czerwinska (Lionbridge Game Services)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Shinjiro Azumi (Lionbridge Game Services)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_GAMES_TEST_SPECIALISTS_KOREAN, IDS_NONE, ECreditTextTypes_Role},
    {L"Youtaek Park (Lionbridge Game Services)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_FRDRIQUE_FOURNY, IDS_NONE, ECreditTextTypes_Text},
    {L"Sean Buchanan (LOFT9 Business Services)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Jiyoung Lee (LOFT9 Business Services)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Mike Jennings (LOFT9 Business Services)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Mark Thomas (LOFT9 Business Services)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Kevin Han (LOFT9 Business Services)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Atsushi Horiuchi", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Alfonsina Mossello (Keywords International)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Jerome Terroir (Keywords International)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Carlotta Bergoglio (Binari Sonori)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Giuseppe Barra (Binari Sonori)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Paolo Pirocchi (Binari Sonori)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Mariella Baldussi (Binari Sonori)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_SPECIAL_THANKS, IDS_NONE, ECreditTextTypes_Role},
    {L"David Boker", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Jared Huffman", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Jeremy Hall", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Jimmy Bischoff", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Matt Booty", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Helen Chiang", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Marcus Purvis", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Phil Spencer", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Robin Vincent", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Saxs Persson", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Stephanie Bonnington", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Lionbridge Game Services", IDS_NONE, IDS_NONE, ECreditTextTypes_Title},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_MICROSOFT_PROJECT_LEAD, IDS_NONE, ECreditTextTypes_Role},
    {L"Jonathan Stacey (Compliance)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_UK_TEST_TEAM_LEAD, IDS_NONE, ECreditTextTypes_Role},
    {L"Aidan Bower (Compliance)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_WAW_TEST_TEAM_LEAD, IDS_NONE, ECreditTextTypes_Role},
    {L"Kamil Bazydlo (Compliance)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_TEST_MANAGER, IDS_NONE, ECreditTextTypes_Role},
    {L"%ls", IDS_CREDITS_BAEJ_ZAWADZKI, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_PROJECT_MANAGER, IDS_NONE, ECreditTextTypes_Role},
    {L"Mariusz Gelnicki", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_MICHA_BOSKI, IDS_NONE, ECreditTextTypes_Text},
    {L"Grzegorz Kohorewicz ", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_TEST_LEADS, IDS_NONE, ECreditTextTypes_Role},
    {L"%ls", IDS_CREDITS_MATEUSZ_SZYMASKI, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_SENIOR_SOFTWARE_TEST_ENGINEER, IDS_NONE, ECreditTextTypes_Role},
    {L"James Small", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_SOFTWARE_TEST_ENGINEERS, IDS_NONE, ECreditTextTypes_Role},
    {L"Aleksander Kopania", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_MACIEJ_ZWIERZYSKI, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_DAMIAN_GABRY, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_KONRAD_MONTWI, IDS_NONE, ECreditTextTypes_Text},
    {L"Mohammed Ajaib (Compliance)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Tom French", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Nick Dorsett (Compliance)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Ryszard Kowalczyk (Compliance)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Lukasz Mikusek (Compliance)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_MICHA_WACISKI, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_WADYSAW_SZAKIEWICZ, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_AGATA_JABOSKA, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_PRZEMYSAW_PDZIERSKI, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_LIONBRIDGE_OPERATIONS_MANAGER, IDS_NONE, ECreditTextTypes_Role},
    {L"Wesley Osborne", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_TEST_ASSOCIATES, IDS_NONE, ECreditTextTypes_Role},
    {L"%ls", IDS_CREDITS_PAWE_DERMANOWSKI_COMPLIANCE, IDS_NONE, ECreditTextTypes_Text},
    {L"Kamil Zakrzewski (Compliance)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_PIOTR_OROWSKI_COMPLIANCE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_ANNA_PONOMARW_COMPLIANCE, IDS_NONE, ECreditTextTypes_Text},
    {L"Adam Ronowski (Compliance)", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_KRZYSZTOF_UCZAK_COMPLIANCE, IDS_NONE, ECreditTextTypes_Text},
    {L"Adrian Rozenek", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_ARKADIUSZ_KOS, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_BARBARA_UKASZEWICZ, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_BARTOMIEJ_KMITA, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_BARTOMIEJ_NOWAKOWSKI, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_DAMIAN_STPIE, IDS_NONE, ECreditTextTypes_Text},
    {L"Jakub Bohdan", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Katarzyna Smyk", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Karol Kolbus", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_MACIEJ_URO, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_MARCIN_KULPISKI, IDS_NONE, ECreditTextTypes_Text},
    {L"Mariusz Zielenkiewicz", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_MICHA_WYROLAK, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_MICHA_SAWEK, IDS_NONE, ECreditTextTypes_Text},
    {L"Monika Chmielewska", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_PAWE_ROMANOWSKI, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_RADOSAW_PARFIENIUK, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_RAFA_WOJDA, IDS_NONE, ECreditTextTypes_Text},
    {L"Wojciech Nieckarz", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Wojciech Pokrzywa", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_ADAM_GIERMASZ, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_ANNA_PONOMARW, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_ANNA_SZAFRAN, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_BARBARA_UKASZEWICZ, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_FILIP_SARNOWSKI, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_JAN_ZOZMAN, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_KAMIL_DRZEWIECKI, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_JAN_DERLUKIEWICZ, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_KAMIL_WASIKOWSKI, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_JULIA_WASTOWSKA, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_ANNA_WRBEL, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_ARKADIUSZ_SYKUA, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_BARTOMIEJ_AREK, IDS_NONE, ECreditTextTypes_Text},
    {L"Emil Grzyb", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Jakub Malinowski", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Jakub Pigul", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Jan Prejs", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Krzysztof Zalewski", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Lazaros Ananiadis", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Mariusz Gil", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Mateusz Kroczek", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_MICHA_BAJGUZ, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_MICHA_OLESZEK, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_MICHA_STEFASKI, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_PRZEMYSAW_WRBEL, IDS_NONE, ECreditTextTypes_Text},
    {L"Rajmund Krakowski", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_LOCALIZATION, IDS_NONE, ECreditTextTypes_Role},
    {L"Francisco Reyes", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Katarzyna Druszcz", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_BAEJ_ZAWADZKI, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_RAFA_NDZAREK, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_MICHA_WOJECIECHOWSKI, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_WOJCIECH_OHOROK, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_MICHA_BOROWICZ, IDS_NONE, ECreditTextTypes_Text},
    {L"Simone Toccacieli", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_PAUL_MORRIS, IDS_NONE, ECreditTextTypes_Text},
    {L"Special Thanks", IDS_NONE, IDS_NONE, ECreditTextTypes_Role},
    {L"David Hickey", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_ADAM_KEATING, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_JERZY_TYMISKI, IDS_NONE, ECreditTextTypes_Text},
    {L"%ls", IDS_CREDITS_PAULINA_LIWISKA, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Nintendo Properties: (c) Nintendo.", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Special thanks to Nintendo", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", CREDIT_IMAGE, UIScene_Credits::ECreditIcons_Nintendo, ECreditTextTypes_Text},
    {L"", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"", CREDIT_IMAGE, UIScene_Credits::ECreditIcons_Miles, ECreditTextTypes_Text},
    {L"Uses Miles Sound System.", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
    {L"Copyright (C) 1991-2014 by RAD Game Tools, Inc.", IDS_NONE, IDS_NONE, ECreditTextTypes_Text},
};
}

UIScene_Credits::UIScene_Credits(int type, void*, UILayer* layer) : UIScene(type, layer)
{
    initialiseMovie();
    m_requestMoreData = false;
    m_creditCount = CREDIT_TEXT_COUNT;

    unsigned int packCount = gApp.mDLCManager.getPackCount(DLCManager::eDLCType_All);
    for (unsigned int packIndex = 0; packIndex < packCount; ++packIndex)
    {
        DLCPack* pack = gApp.mDLCManager.getPack(packIndex, DLCManager::eDLCType_All);
        std::vector<std::wstring>& creditIds = pack->m_creditIds;
        if (!creditIds.empty())
        {
            if (pack->doesPackContainFile(DLCManager::eDLCType_Localisation, L"localisation.loc", true, true))
            {
                DLCLocalisationFile* file = static_cast<DLCLocalisationFile*>(
                    pack->getFile(DLCManager::eDLCType_Localisation, L"localisation.loc", true, true));
                StringTable* stringTable = file->getStringTable();
                for (std::vector<std::wstring>::iterator it = creditIds.begin(); it != creditIds.end(); ++it)
                {
                    addCreditLine(stringTable->getString(*it));
                }
            }
            else if (pack->doesPackContainFile(DLCManager::eDLCType_Localisation, L"languages.loc", true, true))
            {
                DLCLocalisationFile* file = static_cast<DLCLocalisationFile*>(
                    pack->getFile(DLCManager::eDLCType_Localisation, L"languages.loc", true, true));
                StringTable* stringTable = file->getStringTable();
                for (std::vector<std::wstring>::iterator it = creditIds.begin(); it != creditIds.end(); ++it)
                {
                    addCreditLine(stringTable->getString(*it));
                }
            }
        }
        std::vector<std::wstring>& creditStrings = pack->m_creditStrings;
        if (!creditStrings.empty())
        {
            for (std::vector<std::wstring>::iterator it = creditStrings.begin(); it != creditStrings.end(); ++it)
            {
                addCreditLine(*it);
            }
        }

        std::vector<DLCPack*>& childPacks = pack->getChildPacks();
        for (std::vector<DLCPack*>::iterator child = childPacks.begin(); child != childPacks.end(); ++child)
        {
            std::vector<std::wstring>& childCreditIds = (*child)->m_creditIds;
            if (!childCreditIds.empty())
            {
                if (pack->doesPackContainFile(DLCManager::eDLCType_Localisation, L"localisation.loc", true, true))
                {
                    DLCLocalisationFile* file = static_cast<DLCLocalisationFile*>(
                        pack->getFile(DLCManager::eDLCType_Localisation, L"localisation.loc", true, true));
                    StringTable* stringTable = file->getStringTable();
                    for (std::vector<std::wstring>::iterator it = childCreditIds.begin(); it != childCreditIds.end();
                         ++it)
                    {
                        addCreditLine(stringTable->getString(*it));
                    }
                }
                else if (pack->doesPackContainFile(DLCManager::eDLCType_Localisation, L"languages.loc", true, true))
                {
                    DLCLocalisationFile* file = static_cast<DLCLocalisationFile*>(
                        pack->getFile(DLCManager::eDLCType_Localisation, L"languages.loc", true, true));
                    StringTable* stringTable = file->getStringTable();
                    for (std::vector<std::wstring>::iterator it = childCreditIds.begin(); it != childCreditIds.end();
                         ++it)
                    {
                        addCreditLine(stringTable->getString(*it));
                    }
                }
            }
            std::vector<std::wstring>& childCreditStrings = (*child)->m_creditStrings;
            if (!childCreditStrings.empty())
            {
                for (std::vector<std::wstring>::iterator it = childCreditStrings.begin();
                     it != childCreditStrings.end(); ++it)
                {
                    addCreditLine(*it);
                }
            }
        }
    }
    m_creditCount += GetDLCCreditsCount();

    m_currentCredit = -1;
    for (int credit = 0; credit < PRELOADED_CREDIT_COUNT; ++credit)
    {
        ++m_currentCredit;
        if (CREDIT_TEXT[credit].primaryId == IDS_NONE)
        {
            setNextLabel(CREDIT_TEXT[credit].text, CREDIT_TEXT[credit].type);
        }
        else
        {
            wchar_t* label = new wchar_t[PRELOADED_LABEL_LENGTH];
            if (CREDIT_TEXT[credit].secondaryId != IDS_NONE)
            {
                swprintf(label, PRELOADED_LABEL_LENGTH, CREDIT_TEXT[credit].text,
                         L10N::GetString(CREDIT_TEXT[credit].primaryId),
                         L10N::GetString(CREDIT_TEXT[credit].secondaryId));
            }
            else
            {
                swprintf(label, PRELOADED_LABEL_LENGTH, CREDIT_TEXT[credit].text,
                         L10N::GetString(CREDIT_TEXT[credit].primaryId));
            }
            setNextLabel(label, CREDIT_TEXT[credit].type);
            delete[] label;
        }
    }
}

bool UIScene_Credits::AlreadySeenCreditText(const std::wstring& text)
{
    for (unsigned int index = 0; index < m_seenCreditText.size(); ++index)
    {
        std::wstring seen = m_seenCreditText.at(index);
        if (seen.compare(text) == 0)
            return true;
    }
    m_seenCreditText.push_back(std::wstring(text.c_str()));
    return false;
}

void UIScene_Credits::AddCreditText(const wchar_t* text)
{
    SCreditTextItemDef* item = new SCreditTextItemDef;
    item->type = ECreditTextTypes_Text;
    item->primaryId = IDS_NONE;
    item->secondaryId = IDS_NONE;
    item->text = new wchar_t[std::wcslen(text) + 1];
    std::wcscpy(const_cast<wchar_t*>(item->text), text);
    m_dlcCredits.push_back(item);
}

void UIScene_Credits::addCreditLine(const std::wstring& text)
{
    if (AlreadySeenCreditText(text))
        return;

    AddCreditText(L"");
    bool useShortLine = !gRenderer->IsHiDef() && !gRenderer->IsWidescreen();
    unsigned int language = XGetLanguage();
    unsigned int maximum;
    switch (language)
    {
    case XC_LANGUAGE_JAPANESE:
    case XC_LANGUAGE_KOREAN:
    case XC_LANGUAGE_TCHINESE:
        maximum = 55;
        break;
    default:
        maximum = useShortLine ? 45 : 55;
        break;
    }
    std::wstring remaining = text;
    while (remaining.size() > maximum)
    {
        unsigned int length = 1;
        while (length < remaining.size() && length + 1 <= maximum)
            ++length;
        std::wstring::size_type split = remaining.find_last_of(L" ", length);
        if (split == std::wstring::npos)
            split = maximum;
        if (split != length)
            ++split;
        AddCreditText(remaining.substr(0, split).c_str());
        remaining = remaining.substr(split);
    }
    AddCreditText(remaining.c_str());
}

int UIScene_Credits::GetDLCCreditsCount()
{
    return static_cast<int>(m_dlcCredits.size());
}

void UIScene_Credits::setNextLabel(const std::wstring& text, ECreditTextTypes type)
{
    static_cast<FJ_CreditsMenu*>(m_movie->getRootNode()->getFuiNode())
        ->SetNextLabel(text, type, m_currentCredit == m_creditCount - 1);
}

void UIScene_Credits::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_playerId, -1, IDS_TOOLTIPS_BACK, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, false, false);
}

void UIScene_Credits::updateComponents()
{
    m_layer->showComponent(m_playerId, EUIScene_Logo, true);
}

void UIScene_Credits::handleReload()
{
    navigateBack();
}

SCreditTextItemDef* UIScene_Credits::GetDLCCredits(int index)
{
    return m_dlcCredits.at(static_cast<std::size_t>(index));
}

void UIScene_Credits::addImage(ECreditIcons icon)
{
    static_cast<FJ_CreditsMenu*>(m_movie->getRootNode()->getFuiNode())
        ->AddImage(icon, m_currentCredit == m_creditCount - 1);
}

void UIScene_Credits::tick()
{
    UIScene::tick();
    if (!m_requestMoreData)
        return;
    m_requestMoreData = false;

    if (m_currentCredit + 1 >= m_creditCount)
    {
        m_currentCredit = 0;
    }
    else
    {
        ++m_currentCredit;
    }

    const SCreditTextItemDef* item;
    if (m_currentCredit >= CREDIT_TEXT_COUNT)
    {
        item = GetDLCCredits(m_currentCredit - CREDIT_TEXT_COUNT);
    }
    else
    {
        item = &CREDIT_TEXT[m_currentCredit];
    }

    if (item->text != NULL)
    {
        if (item->primaryId == CREDIT_IMAGE)
        {
            addImage(static_cast<ECreditIcons>(item->secondaryId));
        }
        else
        {
            std::wstring text(item->text);
            if (!gConsoleUIController.UsingBitmapFont())
            {
                text = replaceAll(text, L"\u00A9", L"(C)");
                text = replaceAll(text, L"\u00AE", L"(R)");
                text = replaceAll(text, L"\u2122", L"(TM)");
                text = replaceAll(text, L"\u2013", L"-");
            }

            wchar_t label[LABEL_BUFFER_LENGTH];
            std::memset(label, 0, sizeof(label));
            if (item->primaryId == IDS_NONE)
            {
                std::wcscpy(label, text.c_str());
            }
            else if (item->secondaryId != IDS_NONE)
            {
                swprintf(label, LABEL_LENGTH, text.c_str(), L10N::GetString(item->primaryId),
                         L10N::GetString(item->secondaryId));
            }
            else
            {
                swprintf(label, LABEL_LENGTH, text.c_str(), L10N::GetString(item->primaryId));
            }
            setNextLabel(label, item->type);
        }
    }
}

std::wstring UIScene_Credits::getMoviePath()
{
    return L"Credits";
}

void UIScene_Credits::handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuCancel:
        if (pressed && !repeat)
        {
            navigateBack();
        }
        break;
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuOk:
        sendInputToMovie(action, repeat, pressed, released);
        break;
    }
}

void UIScene_Credits::handleRequestMoreData(int, bool)
{
    m_requestMoreData = true;
}

int UIScene_Credits::getSceneType()
{
    return EUIScene_Credits;
}

void UIScene_Credits::setLogoLanguage(int language)
{
    static_cast<FJ_CreditsMenu*>(m_movie->getRootNode()->getFuiNode())->SetLogoLanguage(language);
}
