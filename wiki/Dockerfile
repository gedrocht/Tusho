FROM php:8.3-apache

RUN apt-get update \
  && apt-get install -y --no-install-recommends curl tar gzip \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /var/www/html

RUN curl -L https://download.dokuwiki.org/src/dokuwiki/dokuwiki-stable.tgz \
  | tar -xz --strip-components=1

COPY seed/pages/ /var/www/html/data/pages/
COPY seed/media/ /var/www/html/data/media/
COPY seed/conf/local.php /var/www/html/conf/local.php

RUN chown -R www-data:www-data /var/www/html
