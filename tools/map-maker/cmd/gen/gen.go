package main

import (
	"encoding/json"
	"os"

	"github.com/sardap/stop-the-mail/tools/map-maker/maker"
)

func main() {
	outPath := os.Args[1]
	assetsPath := os.Args[2]
	file := os.Args[3]
	var genrate maker.Generate
	jsonBytes, _ := os.ReadFile(file)
	json.Unmarshal(jsonBytes, &genrate)
	genrate.Generate(assetsPath, outPath)
}
