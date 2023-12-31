<?xml version="1.0" encoding="UTF-8" ?>

<!--
 * ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-01-21
 * Description : A framed theme for the digiKam html gallery tool.
 *
 * SPDX-FileCopyrightText: 2010 by Ruediger Bente <ruediger dot bente at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================
 -->

<!DOCTYPE stylesheet [
<!ENTITY raquo "&#187;" >
<!ENTITY blank "&#160;" >
]>

<xsl:transform version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:exsl="http://exslt.org/common"
	extension-element-prefixes="exsl">

<!-- ********************************************************************* -->
<!-- ** Create single image page for each imag                          ** -->
<!-- ********************************************************************* -->
<xsl:template name="createImagePage">
	<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
		<title><xsl:value-of select="title"/></title>
		<link rel="stylesheet" type="text/css" href="../frames/style.css"/>
	</head>
	<body id="imagePage">
	<h1>
		<span>
			<xsl:value-of select="../name"/>
		</span>
	</h1>
	<div id="imagePage">
		<table border="0" width="100%">
		<tr>
			<td align="center">
				<img src="{full/@fileName}" width="{full/@width}" height="{full/@height}" />
				<xsl:if test="original/@fileName != ''">
					<p>
					<a href="{original/@fileName}"><xsl:value-of select="$i18nOriginalImage"/></a>
					(<xsl:value-of select="original/@width"/>x<xsl:value-of select="original/@height"/>)
					</p>
				</xsl:if>
´			</td>
		</tr>
		</table>
		<p>
			<xsl:value-of select="description"/>
		</p>

	</div>
	<h1>
		<span>
			&raquo; <xsl:value-of select="title"/>
			(<xsl:value-of select="position()"/>/<xsl:value-of select="last()"/>)
			&raquo;
		</span>
	</h1>
	</body>
	</html>
</xsl:template>

<!-- ********************************************************************* -->
<!-- ** Create thumbnail page for each collection                       ** -->
<!-- ********************************************************************* -->
<xsl:template name="createThumbnailPage">
	<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
		<title><xsl:value-of select="name"/></title>
		<link rel="stylesheet" type="text/css" href="frames/style.css"/>
	</head>
	<body id="collectionPage">
	<div id="content">
	<ul>
			<xsl:variable name="folder" select='fileName'/>
			<xsl:for-each select="image">
				<li>
					<a href="{$folder}/{full/@fileName}.html" target="image">
						<img src="{$folder}/{thumbnail/@fileName}" width="{thumbnail/@width}" height="{thumbnail/@height}"/>
					</a>
					<xsl:value-of select="title"/>
					(<xsl:value-of select="position()"/>/<xsl:value-of select="last()"/>)
				</li>
				<exsl:document href='{$folder}/{full/@fileName}.html'>
					<xsl:call-template name="createImagePage"/>
				</exsl:document>
			</xsl:for-each>
	</ul>
	</div>
	</body>
	</html>
</xsl:template>

<!-- ********************************************************************* -->
<!-- ** Create frameset page if only one collection                     ** -->
<!-- ********************************************************************* -->
<xsl:template name="createFrameSetPage">
<!-- ** create variable tsize for the width of the thumbnails frame            ** -->
<!-- ** add 50 pixel to tsize for the border around the thumbnail              ** -->
	<xsl:variable name="tsize" select="image[1]/thumbnail/@width + 50"/>
	<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />

		<title>
		<xsl:value-of select="name"/>
		</title>
		<link rel="stylesheet" type="text/css" href="frames/style.css"/>
	</head>
	<frameset cols="{$tsize},*" noresize="1" border="0">
		<frame src="thmbs.html" name="mythmbs"/>
		<frame src="blank.html" name="image"/>
	</frameset>
	<exsl:document href="thmbs.html">
		<xsl:call-template name="createThumbnailPage"/>
	</exsl:document>
	</html>
</xsl:template>

<!-- ********************************************************************* -->
<!-- ** Create the collection index page when more than one collection  ** -->
<!-- ********************************************************************* -->
<xsl:template name="createCollectionIndexPage">
	<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
		<title><xsl:value-of select="$i18nCollectionList"/></title>
		<link rel="stylesheet" type="text/css" href="frames/style.css"/>
	</head>
	<body id="collectPage">
	<div id="CollectPageContent">
			<xsl:for-each select="collections/collection">
					<xsl:sort select="name" order="ascending" data-type="text" />
					&blank;
					<a href="Thmbs{fileName}.html" target="mythmbs">
						<xsl:value-of select="name"/>

					</a>
					<xsl:for-each select="image">
						<xsl:choose>
							<xsl:when test="position()=last()">
								(<xsl:value-of select="last()"/>)
							</xsl:when>
						</xsl:choose>
					</xsl:for-each>
				<exsl:document href="Thmbs{fileName}.html">
					<xsl:call-template name="createThumbnailPage"/>
				</exsl:document>
				
			</xsl:for-each>
	</div> <!-- /content -->
	</body>
	</html>
	
</xsl:template>

<!-- ********************************************************************* -->
<!-- ** Create the frameset page when more than one collection          ** -->
<!-- ********************************************************************* -->
<xsl:template name="createCollectionFrameSetPage">
<!-- ** create variable tsize for the width of the thumbnails frame            ** -->
<!-- ** add 50 pixel to tsize for the border around the thumbnail              ** -->
	<xsl:variable name="tsize" select="collections/collection[1]/image[1]/thumbnail/@width + 50"/>
	<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />

		<title>
		<xsl:value-of select="name"/>
		</title>
		<link rel="stylesheet" type="text/css" href="frames/style.css"/>
	</head>
	<frameset rows="5%,*" noresize="1" border="0">
		<frame src="collect.html" name="collection"/>
		<frameset cols="{$tsize},*" noresize="1" border="0">
			<frame src="blank.html" name="mythmbs"/>
			<frame src="blank.html" name="image"/>
		</frameset>
	</frameset>
	</html>
	<exsl:document href="collect.html">
		<xsl:call-template name="createCollectionIndexPage"/>
	</exsl:document>
</xsl:template>

<!-- ********************************************************************* -->
<!-- ** Create a blank page                                             ** -->
<!-- ** as a starting page when more than one collection is used        ** -->
<!-- ********************************************************************* -->
<xsl:template name="createBlankPage">
	<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />

		<title>
		<xsl:value-of select="name"/>
		</title>
		<link rel="stylesheet" type="text/css" href="frames/style.css"/>
	</head>
	<body id="collectionPage">
		<xsl:value-of select="title"/>
	</body>

	</html>
</xsl:template>


<!-- ********************************************************************* -->
<!-- ** the beginning of all                                            ** -->
<!-- ********************************************************************* -->
<xsl:template match="/">
	<xsl:choose>
		<xsl:when test="count(collections/collection) &gt; 1">
			<xsl:call-template name="createCollectionFrameSetPage"/>
		</xsl:when>
		<xsl:otherwise>
			<xsl:for-each select="collections/collection">
				<xsl:call-template name="createFrameSetPage"/>
			</xsl:for-each>
		</xsl:otherwise>
	</xsl:choose>
	<exsl:document href="blank.html">
		<xsl:call-template name="createBlankPage"/>
	</exsl:document>
</xsl:template>

</xsl:transform>
