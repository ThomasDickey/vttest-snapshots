# Created by: Nicola Vitale <nivit@email.it>
# $FreeBSD: head/sysutils/vttest/Makefile 478968 2018-09-04 16:47:01Z nivit $

PORTNAME=	vttest
PORTVERSION=	2019.01.05
CATEGORIES=	sysutils
MASTER_SITES=	ftp://ftp.invisible-island.net/vttest/
DISTNAME=	${PORTNAME}-${PORTVERSION:S,.,,g}

MAINTAINER=	nivit@FreeBSD.org
COMMENT=	Program to test "VT100-compatibility" of terminals

LICENSE=	MIT
LICENSE_FILE=	${WRKSRC}/COPYING

USES=		tar:tgz

GNU_CONFIGURE=	yes

MAKEFILE=	makefile

PLIST_FILES=	bin/vttest man/man1/vttest.1.gz

.include <bsd.port.mk>
