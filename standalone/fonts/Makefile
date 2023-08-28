# Sunclock, draw a clock with local solar and lunar information
# Copyright (C) 2022,2023 Adam Wozniak / GorillaSapiens
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

all: AstroDotBasic.ttf dejavu-fonts-ttf-2.37/LICENSE

clean:
	rm -f astrodotbasic.zip dejavu-fonts-ttf-2.37.tar.bz2
	rm -f AstroDotBasic.ttf AstroDotBasic.png
	rm -f astrodotbasic.zip dejavu-fonts-ttf-2.37.tar
	rm -rf dejavu-fonts-ttf-2.37

astrodotbasic.zip:
	wget https://astrology-symbols.com/font_files/astrodotbasic.zip

AstroDotBasic.ttf: astrodotbasic.zip
	unzip astrodotbasic.zip
	touch AstroDotBasic.ttf

dejavu-fonts-ttf-2.37.tar.bz2:
	wget http://sourceforge.net/projects/dejavu/files/dejavu/2.37/dejavu-fonts-ttf-2.37.tar.bz2

dejavu-fonts-ttf-2.37.tar: dejavu-fonts-ttf-2.37.tar.bz2
	bunzip2 -k dejavu-fonts-ttf-2.37.tar.bz2

dejavu-fonts-ttf-2.37/LICENSE: dejavu-fonts-ttf-2.37.tar
	tar -xvf dejavu-fonts-ttf-2.37.tar
	touch dejavu-fonts-ttf-2.37/LICENSE

