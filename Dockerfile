FROM nginx:stable

RUN addgroup overpass && adduser --home /db --disabled-password --ingroup overpass overpass

RUN apt-get update \
    && apt-get install --no-install-recommends --no-install-suggests -y \
        autoconf \
        automake \
        expat \
        libexpat1-dev \
        g++ \
        libtool \
        m4 \
        make \
        zlib1g \
        zlib1g-dev

COPY . /app/

RUN cd /app/src \
    && autoscan \
    && aclocal \
    && autoheader \
    && libtoolize \
    && automake --add-missing  \
    && autoconf \
    && CXXFLAGS='-g -O0' CFLAGS='-g -O0' ./configure --prefix=/app \
    && make -j $(grep -c ^processor /proc/cpuinfo) install clean \
    && apt-get remove -y \
        autoconf \
        automake \
        libexpat1-dev \
        g++ \
        libtool \
        m4 \
        make \
        zlib1g-dev \
    && apt-get autoremove -y

RUN apt-get install --no-install-recommends --no-install-suggests -y \
        supervisor \
        bash \
        lftp \
        wget \
        bzip2

COPY etc/supervisord.conf /etc/supervisor/conf.d/supervisord.conf

EXPOSE 80
# CMD ["/usr/bin/supervisord", "-c", "/etc/supervisor/conf.d/supervisord.conf"]
CMD ["/app/docker-entrypoint.sh"]