package main

import (
	"database/sql"
	//"encoding/json"
	"fmt"
	"net/http"

	"github.com/gin-gonic/gin"
	_ "github.com/go-sql-driver/mysql" // Import mysql driver
)

func getDatos(c *gin.Context) {
	db, err := sql.Open("mysql", "root:3045905330115@tcp(localhost:3306)/Proyecto")  // Replace with your credentials
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}
	defer db.Close()

	var data []map[string]interface{}

	rows, err := db.Query("SELECT Pid, Nombre, Llamada, Tamanio, FechayHora FROM Dashboard")
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}
	defer rows.Close()

	for rows.Next() {
		var pid int
		var nombre string
		var llamada string
		var tamano int
		var fechayhora string

		err = rows.Scan(&pid, &nombre, &llamada, &tamano, &fechayhora)
		if err != nil {
			c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
			return
		}

		rowMap := map[string]interface{}{
			"Pid":       pid,
			"Nombre":    nombre,
			"Llamada":   llamada,
			"Tamanio":   tamano,
			"FechayHora": fechayhora,
		}
		data = append(data, rowMap)

		// Print each row during iteration for debugging (optional)
		fmt.Printf("Pid: %d, Nombre: %s, Llamada: %s, Tamanio: %d, FechayHora: %s\n", pid, nombre, llamada, tamano, fechayhora)
	}

	if len(data) == 0 {
		c.JSON(http.StatusOK, gin.H{"message": "No data found"})
		return
	}

	// Marshal the data slice into JSON bytes
	//jsonData, err := json.Marshal(data)
	//if err != nil {
	//	c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
	//	return
	//}

	// Return the data as JSON
	c.JSON(http.StatusOK, data)
}

func main() {
	router := gin.Default()

	// CORS Configuration (optional, adjust as needed)
	router.Use(func(c *gin.Context) {
		c.Writer.Header().Set("Access-Control-Allow-Origin", "*")
		c.Writer.Header().Set("Access-Control-Allow-Methods", "GET")
		c.Writer.Header().Set("Access-Control-Allow-Headers", "Content-Type")
		c.Next()
	})

	router.GET("/datos", getDatos)

	fmt.Println("Server listening on port 8080")
	router.Run(":8080")
}
