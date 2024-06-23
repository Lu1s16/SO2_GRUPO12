import React, { useState, useEffect } from 'react';
import { Pie } from 'react-chartjs-2';
import { Line } from 'react-chartjs-2';
import { Chart as ChartJS, ArcElement, registerables } from 'chart.js';

ChartJS.register(...registerables);
//CON ESTA DIRECCION GUIATE
const API_URL_RAM = 'http://192.168.217.129:8080/datos';
const MEMORIA = 3968880000


function App() {
  
  
  const [Tabla, setTabla] = useState([]);
  const [Solicitudes, setSolicitudes] = useState([]);
  const [listaLabels, setDatosLabel] = useState([]);
  const [listaValues, setDatosValues] = useState([])

  const [currentPage, setCurrentPage] = useState(0);
  const itemsPerPage = 10;

  const [currentPage2, setCurrentPage2] = useState(0);
  const itemsPerPage2 = 10;

  useEffect(() => {
    const fetchData = async () => {
      try {
        const responseRam = await fetch(API_URL_RAM);
        if (responseRam.ok) {
          const data = await responseRam.json();
          
          //Guarda todos los procesos sin agruparlos
          const listaProcesos = []

          for (const objeto of data) {
              const nuevoObjeto = {
                  Pid: objeto.Pid,
                  Nombre: objeto.Nombre.trim(),
                  Llamada: objeto.Llamada,
                  Tamanio: objeto.Tamanio,
                  FechayHora: objeto.FechayHora.trim() 
              }
              listaProcesos.push(nuevoObjeto)
          }
          setSolicitudes(listaProcesos)

          
          //Obtener datos agrupados para la memoria y porcentaje
          const groupedData = listaProcesos.reduce((acc, item) => {
          const key = `${item.Pid}-${item.Nombre}`;
          if (!acc[key]) {
              acc[key] = [];
            }
            acc[key].push(item);
            return acc
            
          }, {});

          //lista que almacena los procesos agrupados y guarda el pid, nombre, memoria y porcentaje
          const procesosAgrupados = []
          
          for (const key in groupedData) {
            if (groupedData.hasOwnProperty(key)) {
              let suma_mmap = 0;
              let suma_munmap = 0;
              let nombre = ""
              let pid = ""
              let memoria = 0
              let porcentaje = 0
              

              //console.log(groupedData[key])
              for (const c in groupedData[key]) {
                nombre = groupedData[key][c].Nombre
                pid = groupedData[key][c].Pid.toString()

                //console.log(groupedData[key][c].Tamanio)
                let llamada = groupedData[key][c].Llamada
                if(llamada == "MMAP") {
                  suma_mmap+=groupedData[key][c].Tamanio

                } else {
                  suma_munmap+=groupedData[key][c].Tamanio

                }
                
              
              }
              memoria = suma_mmap - suma_munmap
              if(memoria < 0) {
                memoria = 0
              }
              porcentaje = (memoria * 100)/MEMORIA;
              const nuevoObjeto = {
                  Pid: pid, 
                  Nombre: nombre,
                  Memoria: memoria,
                  Porcentaje: porcentaje
              
              }
              procesosAgrupados.push(nuevoObjeto)

              
              
            }
          }

          

          procesosAgrupados.sort((a, b) => b.Memoria - a.Memoria);

          console.log(procesosAgrupados);
          setTabla(procesosAgrupados)

          //lista que almacena los primeros 10 con mayor porcentaje y otros
          const listaTop = []
          const listaNombresTop = []
          
          let porcentaje_otros = 0;
          
          for(const c in procesosAgrupados) {

            if(c <= 9) {
              const valor = procesosAgrupados[c].Porcentaje 
              const nombre = procesosAgrupados[c].Nombre 
              listaTop.push(valor)
              listaNombresTop.push(nombre)

            } else {
              const valor = procesosAgrupados[c].Porcentaje 
              porcentaje_otros+=valor 

            }

          }

          listaTop.push(porcentaje_otros)
          listaNombresTop.push("otros")
          setDatosLabel(listaNombresTop)
          setDatosValues(listaTop)


          

        } else {
          throw new Error('Error HTTP RAM: ' + responseRam.status);
        }
      } catch (error) {
        console.error('Error fetching data:', error);
      }
    };
    const interval = setInterval(fetchData, 1000);
    return () => clearInterval(interval);
  }, []);

  //obtener datos
  
  


  const chartData = {
    labels: listaLabels,
    datasets: [
      {
        label: 'Porcentaje',
        data: listaValues,
        backgroundColor: ['#FF3333', '#FF8D33', "#FFEC33", "#7AFF33", "#33FF83", "#33E6FF", "#3352FF", "#9033FF", "#FF33EC"],
        borderColor: ["black"],
        borderWidth: 1
      }
    ]
  };

  //funciones de paginacion 1
  
  const handlePreviousPage = () => {
    setCurrentPage(prevPage => Math.max(prevPage - 1, 0));
  };

  const handleNextPage = () => {
    setCurrentPage(prevPage => (prevPage + 1) * itemsPerPage < Tabla.length ? prevPage + 1 : prevPage);
  };

  const startIndex = currentPage * itemsPerPage;
  const displayedData = Tabla.slice(startIndex, startIndex + itemsPerPage);

  //funciones de paginacion 2

  const handlePreviousPage2 = () => {
    setCurrentPage2(prevPage => Math.max(prevPage - 1, 0));
  };

  const handleNextPage2 = () => {
    setCurrentPage2(prevPage => (prevPage + 1) * itemsPerPage2 < Solicitudes.length ? prevPage + 1 : prevPage);
  };

  const startIndex2 = currentPage2 * itemsPerPage2;
  const displayedData2 = Solicitudes.slice(startIndex2, startIndex2 + itemsPerPage2);



  return (
    <div className="App">
      <header className="masthead text-center text-white">
        <div className="masthead-content">
          <div className="container px-5">
            <h1 className="masthead-heading mb-0">Proyecto</h1>
            <h2>Estudiante 1: Nataly Saraí Guzmán Duarte - 202001570</h2>
            <h2>Estudiante 2: Luis Enrique Garcia Gutierrez - 202010814</h2>
          </div>
        </div>
        <div className="bg-circle-1 bg-circle"></div>
        <div className="bg-circle-2 bg-circle"></div>
        <div className="bg-circle-3 bg-circle"></div>
      </header>
      <nav className="navbar navbar-expand-lg navbar-dark navbar-custom fixed-top">
        <div className="container px-5">
          <a className="navbar-brand" href="#page-top">Proyecto</a>
          <button className="navbar-toggler" type="button" data-bs-toggle="collapse" data-bs-target="#navbarResponsive" aria-controls="navbarResponsive" aria-expanded="false" aria-label="Toggle navigation"><span className="navbar-toggler-icon"></span></button>
          <div className="collapse navbar-collapse" id="navbarResponsive">
            <ul className="navbar-nav ms-auto">
              <li className="nav-item"><a className="nav-link" href="#dashboa">Dashboard</a></li>
              <li className="nav-item"><a className="nav-link" href="#solicit">Solicitudes</a></li>
            </ul>
          </div>
        </div>
      </nav>
      <section id="dashboa">
        <div className="container px-5">
          <div className="row gx-5 align-items-center">
            <h1><center>Dashboard</center></h1>
            <div className="col-lg-6">
              <div className="p-2">
                <Pie data={chartData} />
              </div>
            </div>
            <div className="col-lg-6">
            <div className="p-2">
                <table className="table table-striped">
                  <thead>
                    <tr>
                      <th scope="col">PID</th>
                      <th scope="col">Nombre</th>
                      <th scope="col">Memoria (Bytes)</th>
                      <th scope="col">Porcentaje</th>
                    </tr>
                  </thead>
                  <tbody>
                    {displayedData.map((item, index) => (
                      <tr key={index}>
                        <td>{item.Pid}</td>
                        <td>{item.Nombre}</td>
                        <td>{item.Memoria}</td>
                        <td>{item.Porcentaje}</td>
                      </tr>
                    ))}
                  </tbody>
                </table>
                <div className="pagination">
                    <button onClick={handlePreviousPage} disabled={currentPage === 0}>Anterior</button>
                    <button onClick={handleNextPage} disabled={(currentPage + 1) * itemsPerPage >= Tabla.length}>Siguiente</button>
                </div>
            </div>
            </div>
          </div>
        </div>
      </section>
      <section id="solicit">
        <div className="container px-5">
          <div className="row gx-5 align-items-center">
            <h1><center>Solicitudes</center></h1>
            <div className="col-lg-6">
            <div className="p-2">
                <table className="table table-striped">
                  <thead>
                    <tr>
                      <th scope="col">PID</th>
                      <th scope="col">Llamada</th>
                      <th scope="col">Tamaño (MB)</th>
                      <th scope="col">Fecha</th>
                    </tr>
                  </thead>
                  <tbody>
                    {displayedData2.map((item, index) => (
                      <tr key={index}>
                        <td>{item.Pid}</td>
                        <td>{item.Llamada}</td>
                        <td>{item.Tamanio}</td>
                        <td>{item.FechayHora}</td>
                      </tr>
                    ))}
                  </tbody>
                </table>
                <div className="pagination">
                    <button onClick={handlePreviousPage2} disabled={currentPage2 === 0}>Anterior</button>
                    <button onClick={handleNextPage2} disabled={(currentPage2 + 1) * itemsPerPage2 >= Solicitudes.length}>Siguiente</button>
                </div>
            </div>
            </div>

          </div>
        </div>
      </section>
    </div>
  );
}

export default App;

