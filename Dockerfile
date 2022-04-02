FROM golang:1.18.0 as map-maker-builder

RUN apt-get update -y \
    && apt-get -y install libx11-dev mesa-common-dev \
    libglu1-mesa-dev libxrandr-dev libxi-dev libxcursor-dev \
    libxinerama-dev libgl1-mesa-dev xorg-dev

WORKDIR /app
COPY ./tools/map-maker/go.mod . 
COPY ./tools/map-maker/go.sum . 
RUN go mod download

COPY ./tools/map-maker .

RUN go build -o main ./cmd/gen

############################################

FROM golang:1.18.0 as builder-builder

WORKDIR /app
COPY ./tools/builder/go.mod . 
COPY ./tools/builder/go.sum . 
RUN go mod download

COPY ./tools/builder/*.go ./
COPY ./tools/builder/assets/*.go ./assets/
COPY ./tools/builder/gbacolour/*.go ./gbacolour/

RUN go build -o main .

##################################################

# Devkit pro image is out of date also deabain is fucked here for some reason
FROM devkitpro/devkitarm:latest as GBA-builder

#Copy colour agg tool
COPY --from=map-maker-builder /app/main /bin/map-maker-builder.exe

#Copy builder
COPY --from=builder-builder /app/main /bin/builder
RUN chmod +x /bin/builder

RUN mkdir /app

WORKDIR /app

COPY ./assets ./assets
COPY ./include ./include
COPY ./src ./src
COPY ./build.toml .
COPY Makefile .

RUN builder build /app /app/build.toml /app/assets /app/build map-maker-builder.exe DEBUG=1 
