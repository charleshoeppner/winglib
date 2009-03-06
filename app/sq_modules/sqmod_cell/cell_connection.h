
// cell_connection.h

#pragma once

class CCellConnection
{
public:

	/// Device tag map
	typedef oexStdMap( sqbind::stdString, _tag_detail* )	t_TagMap;

	enum
	{
		// Limit to number of programs to support
		eMaxProgs = 1024
	};

public:

	/// Default constructor
	CCellConnection();

	/// Destructor
	~CCellConnection()
	{	Destroy(); }

	/// Copy constructor
	CCellConnection( const CCellConnection &r );

	/// Copy operator
	CCellConnection& operator = ( const CCellConnection &r ) { return *this; }

	/// Closes the connection and releases resources
	int Destroy();

	/// Connect to device
	int Connect( const sqbind::stdString &sIp, int nLoadTags );

	/// Returns non-zero if connected
	int IsConnected();

	/// Loads the tag database from the target
	int LoadTags();

	/// Releases tags and resources
	int ReleaseTags();

	/// Returns the size of the type
	int GetTypeSize( int nType );

	/// Returns a string describing the tag type
	oex::oexCSTR GetTypeName( int nType );

	/// Returns the integer type from the type name
	int GetTypeFromName( const sqbind::stdString &sType );

	/// Returns a string representation of the specified item
	oex::oexBOOL GetItemValue( int nType, unsigned char *pData, int nSize, int nBit, sqbind::stdString &sRet );

	/// Parses a tag name into components
	oex::oexBOOL ParseTag( const sqbind::stdString &sTag, sqbind::stdString &sName, int &nProgram, int &nTag, int &nOffset, int &nSize, int &nType, int &nBit );

	/// Read the specified tag
	sqbind::CSqMap ReadTag( const sqbind::stdString &sTag );

	/// Returns a string describing backplane data
	sqbind::stdString GetBackplaneData();

	/// Sets information about the last error
	sqbind::stdString SetLastError( const sqbind::stdString sErr )
	{	return m_sLastError = sErr; }

	/// Returns information about the last error
	sqbind::stdString GetLastError()
	{	return m_sLastError; }

	/// Returns the ip address of the currently connected device
	sqbind::stdString GetIp()
	{	return m_sIp; }

	/// Returns a list of tags to squirrel
	sqbind::CSqMap* tags()
	{	return &m_mapSqTags; }

	/// Creates a map from tag details structure
	sqbind::CSqMap TagToMap( _tag_detail *pTd );

	/// Returns the tag template map
	sqbind::CSqMulti* tmpl()
	{	return &m_mapSqTemplates; }

	/// Verifys values in the squirrel template map
	void VerifyTemplate();

private:

	/// Holds last error information
	sqbind::stdString			m_sLastError;

	/// Connection address
	sqbind::stdString			m_sIp;

	/// Connection informatino
	_comm_header				m_comm;

	/// Path
	_path 						m_path;

	/// Detail tags
	_tag_data					m_tagsDetails;

	/// Program list
	_prog_list					m_prog_list;

	/// Program tags
	oex::TMem< _tag_data >		m_tagsProgram;

	/// Device rack
	CCellRack					m_cRack;

	/// Device backplane
	CCellBackplane				m_cBackplane;

	/// Device services
	CCellServices				m_cServices;

	/// Tag map
	t_TagMap					m_mapTags;

	/// Tag map for squirrel
	sqbind::CSqMap				m_mapSqTags;

	/// Tag templates
	sqbind::CSqMulti			m_mapSqTemplates;

};
