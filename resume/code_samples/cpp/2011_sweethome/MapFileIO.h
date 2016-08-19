#ifndef __MAPFILEIO_HEADER__INCLUDED__
#define __MAPFILEIO_HEADER__INCLUDED__

#include <windows.h>
#include <d3dx9.h>
#include <list>
#include "MsDomXmlLib\XmlLibHelper.h"

class CBackground;
class CLevelMap;
class CTile;
class CTileType;
class CTileTypesDict;
class ICreature;
class CStubCreature;

class CMapFileIO 
{
public: 
	CMapFileIO(LPCWSTR szFile = 0);
	~CMapFileIO();

	void SetFileName(LPCWSTR szFile);
	void SetGetSourceHandle(LPWSTR (*pgetsrc)(LPWSTR szType, LPWSTR szName));
	int SaveMap(const CLevelMap &map) const; 
	int LoadMap(CLevelMap &map) const;
	int LoadTileTypesDict(CTileTypesDict& Dict) const;

	void GetContentPath() const;
private :
	//Loaders
	int LoadMap(CLevelMap &map, LPCWSTR szFileName) const;
	int LoadMap(CLevelMap &map, HANDLE hFile) const;

	CBackground* LoadBackground(XML_MANAGER_HANDLE xml, XML_NODE_HANDLE hNode) const;
	CStubCreature* LoadPlayer(XML_MANAGER_HANDLE xml, XML_NODE_HANDLE hPlrNode) const;
	ICreature* LoadCreature(XML_MANAGER_HANDLE xml, XML_NODE_HANDLE hCreNode) const;
	int LoadCreaturesList(std::list<ICreature*> &Creats, XML_MANAGER_HANDLE xml, XML_NODE_HANDLE hNode) const;
	CTile* LoadTile(CTileTypesDict& Dict, XML_MANAGER_HANDLE xml, XML_NODE_HANDLE hTileNode) const;
	int LoadTileList(std::list<CTile*>& List, CTileTypesDict& Dict, XML_MANAGER_HANDLE xml, XML_NODE_HANDLE hNode) const;

	//Savers
	int SaveMap(const CLevelMap &map, LPCWSTR szFileName) const;
	int SaveMap(const CLevelMap &map, HANDLE hFile) const;

	int SaveBackground(const CBackground& Back, XML_MANAGER_HANDLE xml, XML_NODE_HANDLE hRoot) const;

	int SavePlayer( const ICreature* pPlayer, XML_MANAGER_HANDLE xml, XML_NODE_HANDLE hRoot) const;

	int SaveCreature( const ICreature* pCrt, XML_MANAGER_HANDLE xml, XML_NODE_HANDLE hRoot) const;
	int SaveCreatureList(const std::list<ICreature*>& pCreatures, XML_MANAGER_HANDLE xml, XML_NODE_HANDLE hRoot) const;
	int SaveTile(const CTile& Tile, XML_MANAGER_HANDLE xml, XML_NODE_HANDLE hRoot) const;
	int SaveTileList(const std::list<CTile*>& pTiles, XML_MANAGER_HANDLE xml, XML_NODE_HANDLE hRoot) const;

	int WritePosition(XML_MANAGER_HANDLE xml, XML_NODE_HANDLE parent, const D3DXVECTOR3 &pos) const;

	wchar_t *m_fname; //filename which the class uses to save/load a map
	//pointer to a GetSource function which gives monster's src by it's type and name
	LPWSTR (*m_pgetsrc)(LPWSTR szType, LPWSTR szName);
};

#endif