// =================================================================================================
// Copyright 2003-2009 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
//
// Adobe patent application tracking #P435, entitled 'Unique markers to simplify embedding data of
// one format in a file with a different format', inventors: Sean Parent, Greg Gilley.
// =================================================================================================

#include "public/include/XMP_Environment.h"	// ! This must be the first include!
#include "XMPCore/source/XMPCore_Impl.hpp"

#include "XMPCore/source/XMPMeta.hpp"

#include "public/include/XMP_Version.h"
#include "source/UnicodeInlines.incl_cpp"
#include "source/UnicodeConversions.hpp"
#include "XMP_MD5.h"

#if XMP_DebugBuild
	#include <iostream>
#endif

using namespace std;

#if XMP_WinBuild
	#pragma warning ( disable : 4533 )	// initialization of '...' is skipped by 'goto ...'
	#pragma warning ( disable : 4702 )	// unreachable code
	#pragma warning ( disable : 4800 )	// forcing value to bool 'true' or 'false' (performance warning)
#endif


// *** Use the XMP_PropIsXyz (Schema, Simple, Struct, Array, ...) macros
// *** Add debug codegen checks, e.g. that typical masking operations really work
// *** Change all uses of strcmp and strncmp to XMP_LitMatch and XMP_LitNMatch


// =================================================================================================
// Local Types and Constants
// =========================

static const char * kPacketHeader  = "<?xpacket begin=\"\xEF\xBB\xBF\" id=\"W5M0MpCehiHzreSzNTczkc9d\"?>";
static const char * kPacketTrailer = "<?xpacket end=\"w\"?>";	// ! The w/r is at [size-4].

static const char * kTXMP_SchemaGroup = "XMP_SchemaGroup";

static const char * kRDF_XMPMetaStart = "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"";
static const char * kRDF_XMPMetaEnd   = "</x:xmpmeta>";

static const char * kRDF_RDFStart     = "<rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">";
static const char * kRDF_RDFEnd       = "</rdf:RDF>";

static const char * kRDF_SchemaStart  = "<rdf:Description rdf:about=";
static const char * kRDF_SchemaEnd    = "</rdf:Description>";

static const char * kRDF_StructStart  = "<rdf:Description>";
static const char * kRDF_StructEnd    = "</rdf:Description>";

static const char * kRDF_BagStart     = "<rdf:Bag>";
static const char * kRDF_BagEnd       = "</rdf:Bag>";

static const char * kRDF_SeqStart     = "<rdf:Seq>";
static const char * kRDF_SeqEnd       = "</rdf:Seq>";

static const char * kRDF_AltStart     = "<rdf:Alt>";
static const char * kRDF_AltEnd       = "</rdf:Alt>";

static const char * kRDF_ItemStart    = "<rdf:li>";
static const char * kRDF_ItemEnd      = "</rdf:li>";

static const char * kRDF_ValueStart   = "<rdf:value>";
static const char * kRDF_ValueEnd     = "</rdf:value>";


// =================================================================================================
// Static Variables
// ================


// =================================================================================================
// Local Utilities
// ===============


// -------------------------------------------------------------------------------------------------
// EstimateRDFSize
// ---------------

//  *** Pull the strlen(kXyz) calls into constants.

static size_t
EstimateRDFSize ( const XMP_Node * currNode, XMP_Index indent, size_t indentLen )
{
	size_t outputLen = 2 * (indent*indentLen + currNode->name.size() + 4);	// The property element tags.

	if ( ! currNode->qualifiers.empty() ) {
		// This node has qualifiers, assume it is written using rdf:value and estimate the qualifiers.

		indent += 2;	// Everything else is indented inside the rdf:Description element.
		outputLen += 2 * ((indent-1)*indentLen + strlen(kRDF_StructStart) + 2);	// The rdf:Description tags.
		outputLen += 2 * (indent*indentLen + strlen(kRDF_ValueStart) + 2);		// The rdf:value tags.

		for ( size_t qualNum = 0, qualLim = currNode->qualifiers.size(); qualNum < qualLim; ++qualNum ) {
			const XMP_Node * currQual = currNode->qualifiers[qualNum];
			outputLen += EstimateRDFSize ( currQual, indent, indentLen );
		}

	}

	if ( currNode->options & kXMP_PropValueIsStruct ) {
		indent += 1;
		outputLen += 2 * (indent*indentLen + strlen(kRDF_StructStart) + 2);	// The rdf:Description tags.
	} else if ( currNode->options & kXMP_PropValueIsArray ) {
		indent += 2;
		outputLen += 2 * ((indent-1)*indentLen + strlen(kRDF_BagStart) + 2);		// The rdf:Bag/Seq/Alt tags.
		outputLen += 2 * currNode->children.size() * (strlen(kRDF_ItemStart) + 2);	// The rdf:li tags, indent counted in children.
	} else if ( ! (currNode->options & kXMP_SchemaNode) ) {
		outputLen += currNode->value.size();	// This is a leaf value node.
	}

	for ( size_t childNum = 0, childLim = currNode->children.size(); childNum < childLim; ++childNum ) {
		const XMP_Node * currChild = currNode->children[childNum];
		outputLen += EstimateRDFSize ( currChild, indent+1, indentLen );
	}

	return outputLen;

}	// EstimateRDFSize


// -------------------------------------------------------------------------------------------------
// DeclareOneNamespace
// -------------------

static void
DeclareOneNamespace	( XMP_StringPtr   nsPrefix,
					  XMP_StringPtr   nsURI,
					  XMP_VarString	& usedNS,		// ! A catenation of the prefixes with colons.
					  XMP_VarString & outputStr,
					  XMP_StringPtr   newline,
					  XMP_StringPtr   indentStr,
					  XMP_Index       indent )
{
	XMP_VarString boundedPrefix = ":";
	boundedPrefix += nsPrefix;
	size_t nsPos = usedNS.find ( boundedPrefix );

	if ( nsPos == XMP_VarString::npos ) {

		outputStr += newline;
		for ( ; indent > 0; --indent ) outputStr += indentStr;
		outputStr += "xmlns:";
		outputStr += nsPrefix;
		if (outputStr[outputStr.size ( ) - 1] == ':')
			outputStr[outputStr.size ( ) - 1] = '=';	// Change the colon to =.
		else
			outputStr += '=';
		outputStr += '"';
		outputStr += nsURI;
		outputStr += '"';

		usedNS += nsPrefix;

	}

}	// DeclareOneNamespace


// -------------------------------------------------------------------------------------------------
// DeclareElemNamespace
// --------------------

static void
DeclareElemNamespace ( const XMP_VarString & elemName,
					   XMP_VarString &		 usedNS,
					   XMP_VarString &		 outputStr,
					   XMP_StringPtr		 newline,
					   XMP_StringPtr		 indentStr,
					   XMP_Index			 indent )
{
	size_t colonPos = elemName.find ( ':' );

	if ( colonPos != XMP_VarString::npos ) {
		XMP_VarString nsPrefix ( elemName.substr ( 0, colonPos+1 ) );
		XMP_StringPtr nsURI;
		bool nsFound = sRegisteredNamespaces->GetURI ( nsPrefix.c_str(), &nsURI, 0 );
		XMP_Enforce ( nsFound );
		DeclareOneNamespace ( nsPrefix.c_str(), nsURI, usedNS, outputStr, newline, indentStr, indent );
	}

}	// DeclareElemNamespace


// -------------------------------------------------------------------------------------------------
// DeclareUsedNamespaces
// ---------------------

static void
DeclareUsedNamespaces ( const XMP_Node * currNode,
						XMP_VarString &  usedNS,
						XMP_VarString &	 outputStr,
						XMP_StringPtr	 newline,
						XMP_StringPtr	 indentStr,
						XMP_Index		 indent )
{

	if ( currNode->options & kXMP_SchemaNode ) {
		// The schema node name is the URI, the value is the prefix.
		DeclareOneNamespace ( currNode->value.c_str(), currNode->name.c_str(), usedNS, outputStr, newline, indentStr, indent );
	} else if ( currNode->options & kXMP_PropValueIsStruct ) {
		for ( size_t fieldNum = 0, fieldLim = currNode->children.size(); fieldNum < fieldLim; ++fieldNum ) {
			const XMP_Node * currField = currNode->children[fieldNum];
			DeclareElemNamespace ( currField->name, usedNS, outputStr, newline, indentStr, indent );
		}
	}

	for ( size_t childNum = 0, childLim = currNode->children.size(); childNum < childLim; ++childNum ) {
		const XMP_Node * currChild = currNode->children[childNum];
		DeclareUsedNamespaces ( currChild, usedNS, outputStr, newline, indentStr, indent );
	}

	for ( size_t qualNum = 0, qualLim = currNode->qualifiers.size(); qualNum < qualLim; ++qualNum ) {
		const XMP_Node * currQual = currNode->qualifiers[qualNum];
		DeclareElemNamespace ( currQual->name, usedNS, outputStr, newline, indentStr, indent );
		DeclareUsedNamespaces ( currQual, usedNS, outputStr, newline, indentStr, indent );
	}

}	// DeclareUsedNamespaces

// -------------------------------------------------------------------------------------------------
// EmitRDFArrayTag
// ---------------

enum {
	kIsStartTag = true,
	kIsEndTag   = false
};

static void
EmitRDFArrayTag	( XMP_OptionBits  arrayForm,
				  XMP_VarString & outputStr,
				  XMP_StringPtr	  newline,
				  XMP_StringPtr	  indentStr,
				  XMP_Index		  indent,
				  XMP_Index		  arraySize,
				  bool			  isStartTag )
{
	if ( (! isStartTag) && (arraySize == 0) ) return;

	for ( XMP_Index level = indent; level > 0; --level ) outputStr += indentStr;
	if ( isStartTag ) {
		outputStr += "<rdf:";
	} else {
		outputStr += "</rdf:";
	}

	if ( arrayForm & kXMP_PropArrayIsAlternate ) {
		outputStr += "Alt";
	} else if ( arrayForm & kXMP_PropArrayIsOrdered ) {
		outputStr += "Seq";
	} else {
		outputStr += "Bag";
	}

	if ( isStartTag && (arraySize == 0) ) outputStr += '/';
	outputStr += '>';
	outputStr += newline;

}	// EmitRDFArrayTag


// -------------------------------------------------------------------------------------------------
// AppendNodeValue
// ---------------
//
// Append a property or qualifier value to the output with appropriate XML escaping. The escaped
// characters for elements and attributes are '&', '<', '>', and ASCII controls (tab, LF, CR). In
// addition, '"' is escaped for attributes. For efficiency, this is done in a double loop. The outer
// loop makes sure the whole value is processed. The inner loop does a contiguous unescaped run
// followed by one escaped character (if we're not at the end).
//
// We depend on parsing and SetProperty logic to make sure there are no invalid ASCII controls in
// the XMP values. The XML spec only allows tab, LF, and CR. Others are not even allowed as
// numeric escape sequences.

enum {
	kForAttribute = true,
	kForElement   = false
};

static void
AppendNodeValue ( XMP_VarString & outputStr, const XMP_VarString & value, bool forAttribute )
{

	unsigned char * runStart = (unsigned char *) value.c_str();
	unsigned char * runLimit  = runStart + value.size();
	unsigned char * runEnd;
	unsigned char   ch;

	while ( runStart < runLimit ) {

		for ( runEnd = runStart; runEnd < runLimit; ++runEnd ) {
			ch = *runEnd;
			if ( forAttribute && (ch == '"') ) break;
			if ( (ch < 0x20) || (ch == '&') || (ch == '<') || (ch == '>') ) break;
		}

		outputStr.append ( (char *) runStart, (runEnd - runStart) );

		if ( runEnd < runLimit ) {

			if ( ch < 0x20 ) {

				XMP_Assert ( (ch == kTab) || (ch == kLF) || (ch == kCR) );

				char hexBuf[16];
				memcpy ( hexBuf, "&#xn;", 6 );	// AUDIT: Length of "&#xn;" is 5, hexBuf size is 16.
				hexBuf[3] = kHexDigits[ch&0xF];
				outputStr.append ( hexBuf, 5 );

			} else {

				if ( ch == '"' ) {
					outputStr += "&quot;";
				} else if ( ch == '<' ) {
					outputStr += "&lt;";
				} else if ( ch == '>' ) {
					outputStr += "&gt;";
				} else {
					XMP_Assert ( ch == '&' );
					outputStr += "&amp;";
				}

			}

			++runEnd;

		}

		runStart = runEnd;

	}

}	// AppendNodeValue


// -------------------------------------------------------------------------------------------------
// CanBeRDFAttrProp
// ----------------

static bool
CanBeRDFAttrProp ( const XMP_Node * propNode )
{

	if ( propNode->name[0] == '[' ) return false;
	if ( ! propNode->qualifiers.empty() ) return false;
	if ( propNode->options & kXMP_PropValueIsURI ) return false;
	if ( propNode->options & kXMP_PropCompositeMask ) return false;

	return true;

}	// CanBeRDFAttrProp


// -------------------------------------------------------------------------------------------------
// IsRDFAttrQualifier
// ------------------

static XMP_StringPtr sAttrQualifiers[] = { "xml:lang", "rdf:resource", "rdf:ID", "rdf:bagID", "rdf:nodeID", "" };

static bool
IsRDFAttrQualifier ( XMP_VarString qualName )
{

	for ( size_t i = 0; *sAttrQualifiers[i] != 0; ++i ) {
		if ( qualName == sAttrQualifiers[i] ) return true;
	}

	return false;

}	// IsRDFAttrQualifier


// -------------------------------------------------------------------------------------------------
// StartOuterRDFDescription
// ------------------------
//
// Start the outer rdf:Description element, including all needed xmlns attributes. Leave the element
// open so that the compact form can add proprtty attributes.

static void
StartOuterRDFDescription ( const XMP_Node & xmpTree,
						   XMP_VarString &  outputStr,
						   XMP_StringPtr	newline,
						   XMP_StringPtr	indentStr,
						   XMP_Index		baseIndent )
{

	// Begin the outer rdf:Description start tag.

	for ( XMP_Index level = baseIndent+2; level > 0; --level ) outputStr += indentStr;
	outputStr += kRDF_SchemaStart;
	outputStr += '"';
	outputStr += xmpTree.name;
	outputStr += '"';

	// Write all necessary xmlns attributes.

	XMP_VarString usedNS;
	usedNS.reserve ( 400 );	// The predefined prefixes add up to about 320 bytes.
	usedNS = ":xml:rdf:";

	for ( size_t schema = 0, schemaLim = xmpTree.children.size(); schema != schemaLim; ++schema ) {
		const XMP_Node * currSchema = xmpTree.children[schema];
		DeclareUsedNamespaces ( currSchema, usedNS, outputStr, newline, indentStr, baseIndent+4 );
	}

}	// StartOuterRDFDescription

// -------------------------------------------------------------------------------------------------
// SerializeCanonicalRDFProperty
// -----------------------------
//
// Recursively handles the "value" for a node. It does not matter if it is a top level property, a
// field of a struct, or an item of an array.  The indent is that for the property element. An
// xml:lang qualifier is written as an attribute of the property start tag, not by itself forcing
// the qualified property form. The patterns below mostly ignore attribute qualifiers like xml:lang.
// Except for the one struct case, attribute qualifiers don't affect the output form.
//
//	<ns:UnqualifiedSimpleProperty>value</ns:UnqualifiedSimpleProperty>
//
//	<ns:UnqualifiedStructProperty>	(If no rdf:resource qualifier)
//		<rdf:Description>
//			... Fields, same forms as top level properties
//		</rdf:Description>
//	</ns:UnqualifiedStructProperty>
//
//	<ns:ResourceStructProperty rdf:resource="URI"
//		... Fields as attributes
//	>
//
//	<ns:UnqualifiedArrayProperty>
//		<rdf:Bag> or Seq or Alt
//			... Array items as rdf:li elements, same forms as top level properties
//		</rdf:Bag>
//	</ns:UnqualifiedArrayProperty>
//
//	<ns:QualifiedProperty>
//		<rdf:Description>
//			<rdf:value> ... Property "value" following the unqualified forms ... </rdf:value>
//			... Qualifiers looking like named struct fields
//		</rdf:Description>
//	</ns:QualifiedProperty>

enum { kUseCanonicalRDF = true, kUseAdobeVerboseRDF = false };
enum { kEmitAsRDFValue = true, kEmitAsNormalValue = false };

static void
SerializeCanonicalRDFProperty ( const XMP_Node * propNode,
								XMP_VarString &  outputStr,
								XMP_StringPtr	 newline,
								XMP_StringPtr	 indentStr,
								XMP_Index		 indent,
								bool			 useCanonicalRDF,
								bool			 emitAsRDFValue )
{
	XMP_Index level;
	bool emitEndTag   = true;
	bool indentEndTag = true;

	XMP_OptionBits propForm = propNode->options & kXMP_PropCompositeMask;

	// ------------------------------------------------------------------------------------------
	// Determine the XML element name. Open the start tag with the name and attribute qualifiers.

	XMP_StringPtr elemName = propNode->name.c_str();
	if ( emitAsRDFValue ) {
		elemName= "rdf:value";
	} else if ( *elemName == '[' ) {
		elemName = "rdf:li";
	}

	for ( level = indent; level > 0; --level ) outputStr += indentStr;
	outputStr += '<';
	outputStr += elemName;

	bool hasGeneralQualifiers = false;
	bool hasRDFResourceQual   = false;

	for ( size_t qualNum = 0, qualLim = propNode->qualifiers.size(); qualNum < qualLim; ++qualNum ) {
		const XMP_Node * currQual = propNode->qualifiers[qualNum];
		if ( ! IsRDFAttrQualifier ( currQual->name ) ) {
			hasGeneralQualifiers = true;
		} else {
			if ( currQual->name == "rdf:resource" ) hasRDFResourceQual = true;
			if ( ! emitAsRDFValue ) {
				outputStr += ' ';
				outputStr += currQual->name;
				outputStr += "=\"";
				AppendNodeValue ( outputStr, currQual->value, kForAttribute );
				outputStr += '"';
			}
		}
	}

	// --------------------------------------------------------
	// Process the property according to the standard patterns.

	if ( hasGeneralQualifiers && (! emitAsRDFValue) ) {

		// -----------------------------------------------------------------------------------------
		// This node has general, non-attribute, qualifiers. Emit using the qualified property form.
		// ! The value is output by a recursive call ON THE SAME NODE with emitAsRDFValue set.

		if ( hasRDFResourceQual ) {
			XMP_Throw ( "Can't mix rdf:resource and general qualifiers", kXMPErr_BadRDF );
		}

		if ( ! useCanonicalRDF ) {
			outputStr += " rdf:parseType=\"Resource\">";
			outputStr += newline;
		} else {
			outputStr += '>';
			outputStr += newline;
			indent += 1;
			for ( level = indent; level > 0; --level ) outputStr += indentStr;
			outputStr += "<rdf:Description>";
			outputStr += newline;
		}

		SerializeCanonicalRDFProperty ( propNode, outputStr, newline, indentStr, indent+1,
										useCanonicalRDF, kEmitAsRDFValue );

		for ( size_t qualNum = 0, qualLim = propNode->qualifiers.size(); qualNum < qualLim; ++qualNum ) {
			const XMP_Node * currQual = propNode->qualifiers[qualNum];
			if ( IsRDFAttrQualifier ( currQual->name ) ) continue;
			SerializeCanonicalRDFProperty ( currQual, outputStr, newline, indentStr, indent+1,
											useCanonicalRDF, kEmitAsNormalValue );
		}

		if ( useCanonicalRDF ) {
			for ( level = indent; level > 0; --level ) outputStr += indentStr;
			outputStr += "</rdf:Description>";
			outputStr += newline;
			indent -= 1;
		}

	} else {

		// --------------------------------------------------------------------
		// This node has no general qualifiers. Emit using an unqualified form.

		if ( propForm == 0 ) {

			// --------------------------
			// This is a simple property.

			if ( propNode->options & kXMP_PropValueIsURI ) {
				outputStr += " rdf:resource=\"";
				AppendNodeValue ( outputStr, propNode->value, kForAttribute );
				outputStr += "\"/>";
				outputStr += newline;
				emitEndTag = false;
			} else if ( propNode->value.empty() ) {
				outputStr += "/>";
				outputStr += newline;
				emitEndTag = false;
			} else {
				outputStr += '>';
				AppendNodeValue ( outputStr, propNode->value, kForElement );
				indentEndTag = false;
			}

		} else if ( propForm & kXMP_PropValueIsArray ) {

			// This is an array.
			outputStr += '>';
			outputStr += newline;
			EmitRDFArrayTag ( propForm, outputStr, newline, indentStr, indent+1, static_cast<XMP_Index>(propNode->children.size()), kIsStartTag );
			if ( XMP_ArrayIsAltText(propNode->options) ) NormalizeLangArray ( (XMP_Node*)propNode );
			for ( size_t childNum = 0, childLim = propNode->children.size(); childNum < childLim; ++childNum ) {
				const XMP_Node * currChild = propNode->children[childNum];
				SerializeCanonicalRDFProperty ( currChild, outputStr, newline, indentStr, indent+2,
												useCanonicalRDF, kEmitAsNormalValue );
			}
			EmitRDFArrayTag ( propForm, outputStr, newline, indentStr, indent+1, static_cast<XMP_Index>(propNode->children.size()), kIsEndTag );


		} else if ( ! hasRDFResourceQual ) {

			// This is a "normal" struct, use the nested field element form form.
			XMP_Assert ( propForm & kXMP_PropValueIsStruct );
			if ( propNode->children.size() == 0 ) {
				if ( ! useCanonicalRDF ) {
					outputStr += " rdf:parseType=\"Resource\"/>";
					outputStr += newline;
					emitEndTag = false;
				} else {
					outputStr += '>';
					outputStr += newline;
					for ( level = indent+1; level > 0; --level ) outputStr += indentStr;
					outputStr += "<rdf:Description/>";
					outputStr += newline;
				}
			} else {
				if ( ! useCanonicalRDF ) {
					outputStr += " rdf:parseType=\"Resource\">";
					outputStr += newline;
				} else {
					outputStr += '>';
					outputStr += newline;
					indent += 1;
					for ( level = indent; level > 0; --level ) outputStr += indentStr;
					outputStr += "<rdf:Description>";
					outputStr += newline;
				}
				for ( size_t childNum = 0, childLim = propNode->children.size(); childNum < childLim; ++childNum ) {
					const XMP_Node * currChild = propNode->children[childNum];
					SerializeCanonicalRDFProperty ( currChild, outputStr, newline, indentStr, indent+1,
													useCanonicalRDF, kEmitAsNormalValue );
				}
				if ( useCanonicalRDF ) {
					for ( level = indent; level > 0; --level ) outputStr += indentStr;
					outputStr += "</rdf:Description>";
					outputStr += newline;
					indent -= 1;
				}
			}

		} else {

			// This is a struct with an rdf:resource attribute, use the "empty property element" form.
			XMP_Assert ( propForm & kXMP_PropValueIsStruct );
			for ( size_t childNum = 0, childLim = propNode->children.size(); childNum < childLim; ++childNum ) {
				const XMP_Node * currChild = propNode->children[childNum];
				if ( ! CanBeRDFAttrProp ( currChild ) ) {
					XMP_Throw ( "Can't mix rdf:resource and complex fields", kXMPErr_BadRDF );
				}
				outputStr += newline;
				for ( level = indent+1; level > 0; --level ) outputStr += indentStr;
				outputStr += ' ';
				outputStr += currChild->name;
				outputStr += "=\"";
				outputStr += currChild->value;
				outputStr += '"';
			}
			outputStr += "/>";
			outputStr += newline;
			emitEndTag = false;

		}

	}

	// ----------------------------------
	// Emit the property element end tag.

	if ( emitEndTag ) {
		if ( indentEndTag ) for ( level = indent; level > 0; --level ) outputStr += indentStr;
		outputStr += "</";
		outputStr += elemName;
		outputStr += '>';
		outputStr += newline;
	}

}	// SerializeCanonicalRDFProperty


// -------------------------------------------------------------------------------------------------
// SerializeCanonicalRDFSchemas
// ----------------------------
//
// Each schema's properties are written to the single rdf:Description element. All of the necessary
// namespaces are declared in the rdf:Description element. The baseIndent is the base level for the
// entire serialization, that of the x:xmpmeta element. An xml:lang qualifier is written as an
// attribute of the property start tag, not by itself forcing the qualified property form.
//
//	<rdf:Description rdf:about="TreeName"
//			xmlns:ns="URI" ... >
//
//		... The actual properties of the schema, see SerializeCanonicalRDFProperty
//
//		<!-- ns1:Alias is aliased to ns2:Actual -->  ... If alias comments are wanted
//
//	</rdf:Description>

static void
SerializeCanonicalRDFSchemas ( const XMP_Node & xmpTree,
							   XMP_VarString &	outputStr,
							   XMP_StringPtr	newline,
							   XMP_StringPtr	indentStr,
							   XMP_Index		baseIndent,
							   bool				useCanonicalRDF )
{

	StartOuterRDFDescription ( xmpTree, outputStr, newline, indentStr, baseIndent );

	if ( xmpTree.children.size() > 0 ) {
		outputStr += ">";
		outputStr += newline;
	} else {
		outputStr += "/>";
		outputStr += newline;
		return;	// ! Done if there are no XMP properties.
	}

	for ( size_t schemaNum = 0, schemaLim = xmpTree.children.size(); schemaNum < schemaLim; ++schemaNum ) {
		const XMP_Node * currSchema = xmpTree.children[schemaNum];
		for ( size_t propNum = 0, propLim = currSchema->children.size(); propNum < propLim; ++propNum ) {
			const XMP_Node * currProp = currSchema->children[propNum];
			SerializeCanonicalRDFProperty ( currProp, outputStr, newline, indentStr, baseIndent+3,
											useCanonicalRDF, kEmitAsNormalValue );
		}
	}

	// Write the rdf:Description end tag.
	for ( XMP_Index level = baseIndent+2; level > 0; --level ) outputStr += indentStr;
	outputStr += kRDF_SchemaEnd;
	outputStr += newline;

}	// SerializeCanonicalRDFSchemas


// -------------------------------------------------------------------------------------------------
// SerializeCompactRDFAttrProps
// ----------------------------
//
// Write each of the parent's simple unqualified properties as an attribute. Returns true if all
// of the properties are written as attributes.

static bool
SerializeCompactRDFAttrProps ( const XMP_Node *	parentNode,
							   XMP_VarString &	outputStr,
							   XMP_StringPtr	newline,
							   XMP_StringPtr	indentStr,
							   XMP_Index		indent )
{
	size_t prop, propLim;
	bool allAreAttrs = true;

	for ( prop = 0, propLim = parentNode->children.size(); prop != propLim; ++prop ) {

		const XMP_Node * currProp = parentNode->children[prop];
		if ( ! CanBeRDFAttrProp ( currProp ) ) {
			allAreAttrs = false;
			continue;
		}

		outputStr += newline;
		for ( XMP_Index level = indent; level > 0; --level ) outputStr += indentStr;
		outputStr += currProp->name;
		outputStr += "=\"";
		AppendNodeValue ( outputStr, currProp->value, kForAttribute );
		outputStr += '"';

	}

	return allAreAttrs;

}	// SerializeCompactRDFAttrProps


// -------------------------------------------------------------------------------------------------
// SerializeCompactRDFElemProps
// ----------------------------
//
// Recursively handles the "value" for a node that must be written as an RDF property element. It
// does not matter if it is a top level property, a field of a struct, or an item of an array.  The
// indent is that for the property element. The patterns bwlow ignore attribute qualifiers such as
// xml:lang, they don't affect the output form.
//
//	<ns:UnqualifiedStructProperty-1
//		... The fields as attributes, if all are simple and unqualified
//	/>
//
//	<ns:UnqualifiedStructProperty-2 rdf:parseType="Resource">
//		... The fields as elements, if none are simple and unqualified
//	</ns:UnqualifiedStructProperty-2>
//
//	<ns:UnqualifiedStructProperty-3>
//		<rdf:Description
//			... The simple and unqualified fields as attributes
//		>
//			... The compound or qualified fields as elements
//		</rdf:Description>
//	</ns:UnqualifiedStructProperty-3>
//
//	<ns:UnqualifiedArrayProperty>
//		<rdf:Bag> or Seq or Alt
//			... Array items as rdf:li elements, same forms as top level properties
//		</rdf:Bag>
//	</ns:UnqualifiedArrayProperty>
//
//	<ns:QualifiedProperty rdf:parseType="Resource">
//		<rdf:value> ... Property "value" following the unqualified forms ... </rdf:value>
//		... Qualifiers looking like named struct fields
//	</ns:QualifiedProperty>

// *** Consider numbered array items, but has compatibility problems.
// *** Consider qualified form with rdf:Description and attributes.

static void
SerializeCompactRDFElemProps ( const XMP_Node *	parentNode,
							   XMP_VarString &	outputStr,
							   XMP_StringPtr	newline,
							   XMP_StringPtr	indentStr,
							   XMP_Index		indent )
{
	XMP_Index level;

	for ( size_t prop = 0, propLim = parentNode->children.size(); prop != propLim; ++prop ) {

		const XMP_Node * propNode = parentNode->children[prop];
		if ( CanBeRDFAttrProp ( propNode ) ) continue;

		bool emitEndTag = true;
		bool indentEndTag = true;

		XMP_OptionBits propForm = propNode->options & kXMP_PropCompositeMask;

		// -----------------------------------------------------------------------------------
		// Determine the XML element name, write the name part of the start tag. Look over the
		// qualifiers to decide on "normal" versus "rdf:value" form. Emit the attribute
		// qualifiers at the same time.

		XMP_StringPtr elemName = propNode->name.c_str();
		if ( *elemName == '[' ) elemName = "rdf:li";

		for ( level = indent; level > 0; --level ) outputStr += indentStr;
		outputStr += '<';
		outputStr += elemName;

		bool hasGeneralQualifiers = false;
		bool hasRDFResourceQual   = false;

		for ( size_t qualNum = 0, qualLim = propNode->qualifiers.size(); qualNum < qualLim; ++qualNum ) {
			const XMP_Node * currQual = propNode->qualifiers[qualNum];
			if ( ! IsRDFAttrQualifier ( currQual->name ) ) {
				hasGeneralQualifiers = true;
			} else {
				if ( currQual->name == "rdf:resource" ) hasRDFResourceQual = true;
				outputStr += ' ';
				outputStr += currQual->name;
				outputStr += "=\"";
				AppendNodeValue ( outputStr, currQual->value, kForAttribute );
				outputStr += '"';
			}
		}

		// --------------------------------------------------------
		// Process the property according to the standard patterns.

		if ( hasGeneralQualifiers ) {

			// -------------------------------------------------------------------------------------
			// The node has general qualifiers, ones that can't be attributes on a property element.
			// Emit using the qualified property pseudo-struct form. The value is output by a call
			// to SerializeCanonicalRDFProperty with emitAsRDFValue set.

			// *** We're losing compactness in the calls to SerializeCanonicalRDFProperty.
			// *** Should refactor to have SerializeCompactRDFProperty that does one node.

			outputStr += " rdf:parseType=\"Resource\">";
			outputStr += newline;

			SerializeCanonicalRDFProperty ( propNode, outputStr, newline, indentStr, indent+1,
											kUseAdobeVerboseRDF, kEmitAsRDFValue );

			size_t qualNum = 0;
			size_t qualLim = propNode->qualifiers.size();
			if ( propNode->options & kXMP_PropHasLang ) ++qualNum;

			for ( ; qualNum < qualLim; ++qualNum ) {
				const XMP_Node * currQual = propNode->qualifiers[qualNum];
				SerializeCanonicalRDFProperty ( currQual, outputStr, newline, indentStr, indent+1,
												kUseAdobeVerboseRDF, kEmitAsNormalValue );
			}

		} else {

			// --------------------------------------------------------------------
			// This node has only attribute qualifiers. Emit as a property element.

			if ( propForm == 0 ) {

				// --------------------------
				// This is a simple property.

				if ( propNode->options & kXMP_PropValueIsURI ) {
					outputStr += " rdf:resource=\"";
					AppendNodeValue ( outputStr, propNode->value, kForAttribute );
					outputStr += "\"/>";
					outputStr += newline;
					emitEndTag = false;
				} else if ( propNode->value.empty() ) {
					outputStr += "/>";
					outputStr += newline;
					emitEndTag = false;
				} else {
					outputStr += '>';
					AppendNodeValue ( outputStr, propNode->value, kForElement );
					indentEndTag = false;
				}

			} else if ( propForm & kXMP_PropValueIsArray ) {

				// -----------------
				// This is an array.

				outputStr += '>';
				outputStr += newline;
				EmitRDFArrayTag ( propForm, outputStr, newline, indentStr, indent+1, static_cast<XMP_Index>(propNode->children.size()), kIsStartTag );

				if ( XMP_ArrayIsAltText(propNode->options) ) NormalizeLangArray ( (XMP_Node*)propNode );
				SerializeCompactRDFElemProps ( propNode, outputStr, newline, indentStr, indent+2 );

				EmitRDFArrayTag ( propForm, outputStr, newline, indentStr, indent+1, static_cast<XMP_Index>(propNode->children.size()), kIsEndTag );

			} else {

				// ----------------------
				// This must be a struct.

				XMP_Assert ( propForm & kXMP_PropValueIsStruct );

				bool hasAttrFields = false;
				bool hasElemFields = false;

				size_t field, fieldLim;
				for ( field = 0, fieldLim = propNode->children.size(); field != fieldLim; ++field ) {
					XMP_Node * currField = propNode->children[field];
					if ( CanBeRDFAttrProp ( currField ) ) {
						hasAttrFields = true;
						if ( hasElemFields ) break;	// No sense looking further.
					} else {
						hasElemFields = true;
						if ( hasAttrFields ) break;	// No sense looking further.
					}
				}

				if ( hasRDFResourceQual && hasElemFields ) {
					XMP_Throw ( "Can't mix rdf:resource qualifier and element fields", kXMPErr_BadRDF );
				}

				if ( propNode->children.size() == 0 ) {

					// Catch an empty struct as a special case. The case below would Q_EMIT an empty
					// XML element, which gets reparsed as a simple property with an empty value.
					outputStr += " rdf:parseType=\"Resource\"/>";
					outputStr += newline;
					emitEndTag = false;

				} else if ( ! hasElemFields ) {

					// All fields can be attributes, use the emptyPropertyElt form.
					SerializeCompactRDFAttrProps ( propNode, outputStr, newline, indentStr, indent+1 );
					outputStr += "/>";
					outputStr += newline;
					emitEndTag = false;

				} else if ( ! hasAttrFields ) {

					// All fields must be elements, use the parseTypeResourcePropertyElt form.
					outputStr += " rdf:parseType=\"Resource\">";
					outputStr += newline;
					SerializeCompactRDFElemProps ( propNode, outputStr, newline, indentStr, indent+1 );

				} else {

					// Have a mix of attributes and elements, use an inner rdf:Description.
					outputStr += '>';
					outputStr += newline;
					for ( level = indent+1; level > 0; --level ) outputStr += indentStr;
					outputStr += "<rdf:Description";
					SerializeCompactRDFAttrProps ( propNode, outputStr, newline, indentStr, indent+2 );
					outputStr += ">";
					outputStr += newline;
					SerializeCompactRDFElemProps ( propNode, outputStr, newline, indentStr, indent+1 );
					for ( level = indent+1; level > 0; --level ) outputStr += indentStr;
					outputStr += kRDF_StructEnd;
					outputStr += newline;

				}

			}

		}

		// ----------------------------------
		// Emit the property element end tag.

		if ( emitEndTag ) {
			if ( indentEndTag ) for ( level = indent; level > 0; --level ) outputStr += indentStr;
			outputStr += "</";
			outputStr += elemName;
			outputStr += '>';
			outputStr += newline;
		}

	}

}	// SerializeCompactRDFElemProps


// -------------------------------------------------------------------------------------------------
// SerializeCompactRDFSchemas
// --------------------------
//
// All properties from all schema are written in a single rdf:Description element, as are all of the
// necessary namespace declarations. The baseIndent is the base level for the entire serialization,
// that of the x:xmpmeta element. The x:xmpmeta and rdf:RDF elements have already been written.
//
// Top level simple unqualified properties are written as attributes of the (only) rdf:Description
// element. Structs, arrays, and qualified properties are written by SerializeCompactRDFElemProp. An
// xml:lang qualifier on a simple property prevents the attribute form.
//
//	<rdf:Description rdf:about="TreeName"
//			xmlns:ns="URI" ...
//		ns:UnqualifiedSimpleProperty="value" ... >
//		... The remaining properties of the schema, see SerializeCompactRDFElemProps
//	</rdf:Description>

static void
SerializeCompactRDFSchemas ( const XMP_Node & xmpTree,
							 XMP_VarString &  outputStr,
							 XMP_StringPtr	  newline,
							 XMP_StringPtr	  indentStr,
							 XMP_Index		  baseIndent )
{
	XMP_Index level;
	size_t schema, schemaLim;

	StartOuterRDFDescription ( xmpTree, outputStr, newline, indentStr, baseIndent );

	// Write the top level "attrProps" and close the rdf:Description start tag.
	bool allAreAttrs = true;
	for ( schema = 0, schemaLim = xmpTree.children.size(); schema != schemaLim; ++schema ) {
		const XMP_Node * currSchema = xmpTree.children[schema];
		allAreAttrs &= SerializeCompactRDFAttrProps ( currSchema, outputStr, newline, indentStr, baseIndent+3 );
	}
	if ( ! allAreAttrs ) {
		outputStr += ">";
		outputStr += newline;
	} else {
		outputStr += "/>";
		outputStr += newline;
		return;	// ! Done if all properties in all schema are written as attributes.
	}

	// Write the remaining properties for each schema.
	for ( schema = 0, schemaLim = xmpTree.children.size(); schema != schemaLim; ++schema ) {
		const XMP_Node * currSchema = xmpTree.children[schema];
		SerializeCompactRDFElemProps ( currSchema, outputStr, newline, indentStr, baseIndent+3 );
	}

	// Write the rdf:Description end tag.
	for ( level = baseIndent+2; level > 0; --level ) outputStr += indentStr;
	outputStr += kRDF_SchemaEnd;
	outputStr += newline;

}	// SerializeCompactRDFSchemas

// -------------------------------------------------------------------------------------------------
// SerializeAsRDF
// --------------
//
//	<?xpacket begin... ?>
//	<x:xmpmeta xmlns:x=... >
//		<rdf:RDF xmlns:rdf=... >
//
//			... The properties, see SerializeCanonicalRDFSchema or SerializeCompactRDFSchemas
//
//		</rdf:RDF>
//	</x:xmpmeta>
//	<?xpacket end... ?>

// *** Need to strip empty arrays?
// *** Option to strip/keep empty structs?
// *** Need to verify handling of rdf:type qualifiers in canonical and compact.
// *** Need to verify round tripping of rdf:ID and similar qualifiers, see RDF 7.2.21.
// *** Check cases of rdf:resource plus explicit attr qualifiers (like xml:lang).

static void
SerializeAsRDF ( const XMPMeta & xmpObj,
				 XMP_VarString & headStr,	// Everything up to the padding.
				 XMP_VarString & tailStr,	// Everything after the padding.
				 XMP_OptionBits	 options,
				 XMP_StringPtr	 newline,
				 XMP_StringPtr	 indentStr,
				 XMP_Index		 baseIndent )
{
	const size_t treeNameLen = xmpObj.tree.name.size();
	const size_t indentLen   = strlen ( indentStr );

	// First estimate the worst case space and reserve room in the output string. This optimization
	// avoids reallocating and copying the output as it grows. The initial count does not look at
	// the values of properties, so it does not account for character entities, e.g. &#xA; for newline.
	// Since there can be a lot of these in things like the base 64 encoding of a large thumbnail,
	// inflate the count by 1/4 (easy to do) to accommodate.

	// *** Need to include estimate for alias comments.

	size_t outputLen = 2 * (strlen(kPacketHeader) + strlen(kRDF_XMPMetaStart) + strlen(kRDF_RDFStart) + 3*baseIndent*indentLen);

	for ( size_t schemaNum = 0, schemaLim = xmpObj.tree.children.size(); schemaNum < schemaLim; ++schemaNum ) {
		const XMP_Node * currSchema = xmpObj.tree.children[schemaNum];
		outputLen += 2*(baseIndent+2)*indentLen + strlen(kRDF_SchemaStart) + treeNameLen + strlen(kRDF_SchemaEnd) + 2;
		outputLen += EstimateRDFSize ( currSchema, baseIndent+2, indentLen );
	}

	outputLen += (outputLen >> 2);	// Inflate by 1/4, an empirical fudge factor.

	// Now generate the RDF into the head string as UTF-8.

	XMP_Index level;

	std::string rdfstring;
	headStr.erase();
	rdfstring.reserve ( outputLen );

	// Write the rdf:RDF start tag.
	rdfstring += kRDF_RDFStart;
	rdfstring += newline;

	// Write all of the properties.
	if ( options & kXMP_UseCompactFormat ) {
		SerializeCompactRDFSchemas ( xmpObj.tree, rdfstring, newline, indentStr, baseIndent );
	} else {
		bool useCanonicalRDF = XMP_OptionIsSet ( options, kXMP_UseCanonicalFormat );
		SerializeCanonicalRDFSchemas ( xmpObj.tree, rdfstring, newline, indentStr, baseIndent, useCanonicalRDF );
	}

	// Write the rdf:RDF end tag.
	for ( level = baseIndent+1; level > 0; --level ) rdfstring += indentStr;
	rdfstring += kRDF_RDFEnd;
	// Write the packet header PI.
	if ( ! (options & kXMP_OmitPacketWrapper) ) {
		for ( level = baseIndent; level > 0; --level ) headStr += indentStr;
		headStr += kPacketHeader;
		headStr += newline;
	}

	// Write the xmpmeta element's start tag.
	if ( ! (options & kXMP_OmitXMPMetaElement) ) {
		for ( level = baseIndent; level > 0; --level ) headStr += indentStr;
		headStr += kRDF_XMPMetaStart;
		headStr += kXMPCore_VersionMessage  "\"";
		std::string digestStr;
		unsigned char digestBin [16];
		if (options & kXMP_IncludeRDFHash)
		{
			std::string hashrdf;
			MD5_CTX    context;
			MD5Init ( &context );
			MD5Update ( &context, (XMP_Uns8*)rdfstring.c_str(), (unsigned int)rdfstring.size() );
			MD5Final ( digestBin, &context );
			char buffer [40];
			for ( int in = 0, out = 0; in < 16; in += 1, out += 2 ) {
				XMP_Uns8 byte = digestBin[in];
				buffer[out]   = kHexDigits [ byte >> 4 ];
				buffer[out+1] = kHexDigits [ byte & 0xF ];
			}
			buffer[32] = 0;
			digestStr.append ( buffer );
			headStr += " rdfhash=\"";
			headStr += digestStr + "\"";
			headStr += " merged=\"0\"";
		}
		headStr += ">";
		headStr += newline;
	}

	for ( level = baseIndent+1; level > 0; --level ) headStr += indentStr;
	headStr+= rdfstring ;
	headStr += newline;

	// Write the xmpmeta end tag.
	if ( ! (options & kXMP_OmitXMPMetaElement) ) {
		for ( level = baseIndent; level > 0; --level ) headStr += indentStr;
		headStr += kRDF_XMPMetaEnd;
		headStr += newline;
	}

	// Write the packet trailer PI into the tail string as UTF-8.
	tailStr.erase();
	if ( ! (options & kXMP_OmitPacketWrapper) ) {
		tailStr.reserve ( strlen(kPacketTrailer) + (strlen(indentStr) * baseIndent) );
		for ( level = baseIndent; level > 0; --level ) tailStr += indentStr;
		tailStr += kPacketTrailer;
		if ( options & kXMP_ReadOnlyPacket ) tailStr[tailStr.size()-4] = 'r';
	}

}	// SerializeAsRDF


// -------------------------------------------------------------------------------------------------
// SerializeToBuffer
// -----------------

void
XMPMeta::SerializeToBuffer ( XMP_VarString * rdfString,
							 XMP_OptionBits	 options,
							 XMP_StringLen	 padding,
							 XMP_StringPtr	 newline,
							 XMP_StringPtr	 indentStr,
							 XMP_Index		 baseIndent ) const
{
	XMP_Enforce( rdfString != 0 );
	XMP_Assert ( (newline != 0) && (indentStr != 0) );
	rdfString->erase();

	// Fix up some default parameters.

	enum { kDefaultPad = 2048 };
	size_t unicodeUnitSize = 1;
	XMP_OptionBits charEncoding = options & kXMP_EncodingMask;

	if ( charEncoding != kXMP_EncodeUTF8 ) {
		if ( options & _XMP_UTF16_Bit ) {
			if ( options & _XMP_UTF32_Bit ) XMP_Throw ( "Can't use both _XMP_UTF16_Bit and _XMP_UTF32_Bit", kXMPErr_BadOptions );
			unicodeUnitSize = 2;
		} else if ( options & _XMP_UTF32_Bit ) {
			unicodeUnitSize = 4;
		} else {
			XMP_Throw ( "Can't use _XMP_LittleEndian_Bit by itself", kXMPErr_BadOptions );
		}
	}

	if ( options & kXMP_OmitAllFormatting ) {
		newline = " ";	// ! Yes, a space for "newline". This ensures token separation.
		indentStr = "";
	} else {
		if ( *newline == 0 ) newline = "\xA";	// Linefeed
		if ( *indentStr == 0 ) {
			indentStr = " ";
			if ( ! (options & kXMP_UseCompactFormat) ) indentStr  = "   ";
		}
	}

	if ( options & kXMP_ExactPacketLength ) {
		if ( options & (kXMP_OmitPacketWrapper | kXMP_IncludeThumbnailPad) ) {
			XMP_Throw ( "Inconsistent options for exact size serialize", kXMPErr_BadOptions );
		}
		if ( (padding & (unicodeUnitSize-1)) != 0 ) {
			XMP_Throw ( "Exact size must be a multiple of the Unicode element", kXMPErr_BadOptions );
		}
	} else if ( options & kXMP_ReadOnlyPacket ) {
		if ( options & (kXMP_OmitPacketWrapper | kXMP_IncludeThumbnailPad) ) {
			XMP_Throw ( "Inconsistent options for read-only packet", kXMPErr_BadOptions );
		}
		padding = 0;
	} else if ( options & kXMP_OmitPacketWrapper ) {
		if ( options & kXMP_IncludeThumbnailPad ) {
			XMP_Throw ( "Inconsistent options for non-packet serialize", kXMPErr_BadOptions );
		}
		padding = 0;
	} else if ( options & kXMP_OmitXMPMetaElement ) {
		if ( options & kXMP_IncludeRDFHash ) {
			XMP_Throw ( "Inconsistent options for x:xmpmeta serialize", kXMPErr_BadOptions );
		}
		padding = 0;
	} else {
		if ( padding == 0 ) {
			padding = static_cast<XMP_StringLen>(kDefaultPad * unicodeUnitSize);
		} else if ( (padding >> 28) != 0 ) {
			XMP_Throw ( "Outrageously large padding size", kXMPErr_BadOptions );	// Bigger than 256 MB.
		}
		if ( options & kXMP_IncludeThumbnailPad ) {
			if ( ! this->DoesPropertyExist ( kXMP_NS_XMP, "Thumbnails" ) ) padding += (10000 * unicodeUnitSize);	// *** Need a better estimate.
		}
	}

	// Serialize as UTF-8, then convert to UTF-16 or UTF-32 if necessary, and assemble with the padding and tail.

	std::string tailStr;

	SerializeAsRDF ( *this, *rdfString, tailStr, options, newline, indentStr, baseIndent );

	if ( charEncoding == kXMP_EncodeUTF8 ) {

		if ( options & kXMP_ExactPacketLength ) {
			size_t minSize = rdfString->size() + tailStr.size();
			if ( minSize > padding ) XMP_Throw ( "Can't fit into specified packet size", kXMPErr_BadSerialize );
			padding -= minSize;	// Now the actual amount of padding to add.
		}

		size_t newlineLen = strlen ( newline );

		if ( padding < newlineLen ) {
			rdfString->append ( padding, ' ' );
		} else {
			padding -= newlineLen;	// Write this newline last.
			while ( padding >= (100 + newlineLen) ) {
				rdfString->append ( 100, ' ' );
				*rdfString += newline;
				padding -= (100 + newlineLen);
			}
			rdfString->append ( padding, ' ' );
			*rdfString += newline;
		}

		*rdfString += tailStr;

	} else {

		// Need to convert the encoding. Swap the UTF-8 into a local string and convert back. Assemble everything.

		XMP_VarString utf8Str, newlineStr;
		bool bigEndian = ((charEncoding & _XMP_LittleEndian_Bit) == 0);

		if ( charEncoding & _XMP_UTF16_Bit ) {

			std::string padStr ( "  " );  padStr[0] = 0;	// Assume big endian.

			utf8Str.swap ( *rdfString );
			ToUTF16 ( (UTF8Unit*)utf8Str.c_str(), utf8Str.size(), rdfString, bigEndian );
			utf8Str.swap ( tailStr );
			ToUTF16 ( (UTF8Unit*)utf8Str.c_str(), utf8Str.size(), &tailStr, bigEndian );

			if ( options & kXMP_ExactPacketLength ) {
				size_t minSize = rdfString->size() + tailStr.size();
				if ( minSize > padding ) XMP_Throw ( "Can't fit into specified packet size", kXMPErr_BadSerialize );
				padding -= minSize;	// Now the actual amount of padding to add (in bytes).
			}

			utf8Str.assign ( newline );
			ToUTF16 ( (UTF8Unit*)utf8Str.c_str(), utf8Str.size(), &newlineStr, bigEndian );
			size_t newlineLen = newlineStr.size();

			if ( padding < newlineLen ) {
				for ( int i = padding/2; i > 0; --i ) *rdfString += padStr;
			} else {
				padding -= newlineLen;	// Write this newline last.
				while ( padding >= (200 + newlineLen) ) {
					for ( int i = 100; i > 0; --i ) *rdfString += padStr;
					*rdfString += newlineStr;
					padding -= (200 + newlineLen);
				}
				for ( int i = padding/2; i > 0; --i ) *rdfString += padStr;
				*rdfString += newlineStr;
			}

			*rdfString += tailStr;

		} else {

			std::string padStr ( "    " );  padStr[0] = padStr[1] = padStr[2] = 0;	// Assume big endian.
			UTF8_to_UTF32_Proc Converter = UTF8_to_UTF32BE;

			if ( charEncoding & _XMP_LittleEndian_Bit ) {
				padStr[0] = ' '; padStr[1] = padStr[2] = padStr[3] = 0;
				Converter = UTF8_to_UTF32LE;
			}

			utf8Str.swap ( *rdfString );
			ToUTF32 ( (UTF8Unit*)utf8Str.c_str(), utf8Str.size(), rdfString, bigEndian );
			utf8Str.swap ( tailStr );
			ToUTF32 ( (UTF8Unit*)utf8Str.c_str(), utf8Str.size(), &tailStr, bigEndian );

			if ( options & kXMP_ExactPacketLength ) {
				size_t minSize = rdfString->size() + tailStr.size();
				if ( minSize > padding ) XMP_Throw ( "Can't fit into specified packet size", kXMPErr_BadSerialize );
				padding -= minSize;	// Now the actual amount of padding to add (in bytes).
			}

			utf8Str.assign ( newline );
			ToUTF32 ( (UTF8Unit*)utf8Str.c_str(), utf8Str.size(), &newlineStr, bigEndian );
			size_t newlineLen = newlineStr.size();

			if ( padding < newlineLen ) {
				for ( int i = padding/4; i > 0; --i ) *rdfString += padStr;
			} else {
				padding -= newlineLen;	// Write this newline last.
				while ( padding >= (400 + newlineLen) ) {
					for ( int i = 100; i > 0; --i ) *rdfString += padStr;
					*rdfString += newlineStr;
					padding -= (400 + newlineLen);
				}
				for ( int i = padding/4; i > 0; --i ) *rdfString += padStr;
				*rdfString += newlineStr;
			}

			*rdfString += tailStr;

		}

	}

}	// SerializeToBuffer

// =================================================================================================
