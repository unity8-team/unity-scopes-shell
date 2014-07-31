#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright (C) 2014 Canonical Ltd
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Authored by: Pete Woods <pete.woods@canonical.com>

import tornado.httpserver
import tornado.ioloop
import tornado.netutil
import tornado.web
import sys

RESPONSE = '''<Response>
<Ip>1.2.3.4</Ip>
<Status>OK</Status>
<CountryCode>GB</CountryCode>
<CountryCode3>GBR</CountryCode3>
<CountryName>United Kingdom</CountryName>
<RegionCode>H2</RegionCode>
<RegionName>Lancashire</RegionName>
<City>Accrington</City>
<ZipPostalCode>BB5</ZipPostalCode>
<Latitude>55.7654</Latitude>
<Longitude>-2.7467</Longitude>
<AreaCode>0</AreaCode>
<TimeZone>Europe/London</TimeZone>
</Response>
'''

class Lookup(tornado.web.RequestHandler):
    def get(self):
        sys.stderr.write('GeoIP location requested\n')
        sys.stderr.flush()

        self.write(RESPONSE)
        self.finish()
        
def new_app():
    application = tornado.web.Application([
        (r"/lookup", Lookup),
    ], gzip=True)
    sockets = tornado.netutil.bind_sockets(0, '127.0.0.1')
    server = tornado.httpserver.HTTPServer(application)
    server.add_sockets(sockets)

    sys.stdout.write('%d\n' % sockets[0].getsockname()[1])
    sys.stdout.flush()

    return application

if __name__ == "__main__":
    application = new_app()
    tornado.ioloop.IOLoop.instance().start()
