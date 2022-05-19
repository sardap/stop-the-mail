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

FROM psarda/devkitproubuntu:2210-nds as GBA-builder

RUN apt-get update -y \
    && apt-get install -y libx11-dev libxcursor-dev libgl1-mesa-dev libfontconfig1-dev

COPY --from=psarda/asepritecliubuntu:2210 /aseprite /aseprite
COPY --from=map-maker-builder /app/main /bin/map-maker-builder.exe
COPY --from=builder-builder /app/main /bin/builder

RUN mkdir /app
WORKDIR /app

ENV ASEPRITE_PATH /aseprite/aseprite

ENTRYPOINT [ "builder" ]
CMD [ "build", "/app", "/app/build.toml", "/app/assets", "/app/build", "map-maker-builder.exe", "DEBUG=1" ]
