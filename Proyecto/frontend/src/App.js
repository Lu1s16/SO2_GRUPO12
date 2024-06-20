import React, { useState, useEffect } from 'react';
import { Pie } from 'react-chartjs-2';
import { Line } from 'react-chartjs-2';
import { Chart as ChartJS, ArcElement, registerables } from 'chart.js';

ChartJS.register(...registerables);

const API_URL_RAM = 'http://192.168.0.17:8080/tiempor/ram';
const API_URL_CPU = 'http://192.168.0.17:8080/tiempor/cpu';

function App() {
  const [data, setData] = useState({
    freeRam: null,
    cpuInfo: null,
  });
  const [chartDatahis, setChartDatahis] = useState({});
  const [chartDatahiscp, setChartDatahiscp] = useState({});

  useEffect(() => {
    const fetchData = async () => {
      try {
        const responseRam = await fetch(API_URL_RAM);
        if (responseRam.ok) {
          const dataRam = await responseRam.json();
          const responseCpu = await fetch(API_URL_CPU);
          if (responseCpu.ok) {
            const dataCpu = await responseCpu.json();
            setData({
              freeRam: dataRam.freeRam,
              cpuInfo: dataCpu,
            });
          } else {
            throw new Error('Error HTTP CPU: ' + responseCpu.status);
          }
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

  useEffect(() => {
    const fetchChartData = async () => {
      try {
        const response = await fetch('http://192.168.0.17:8080/tiempohis/ram');
        const data = await response.json();

        const labels = data.histrams.map(item => item.fech);
        const dataValues = data.histrams.map(item => item.histram);

        setChartDatahis({
          labels,
          datasets: [
            {
              label: 'RAM Usada',
              data: dataValues,
              borderColor: 'orange',
              backgroundColor: 'transparent',
              pointBorderColor: 'orange',
              pointBackgroundColor: 'rgba(255,150,0,0.5)',
              pointRadius: 5,
              pointHoverRadius: 10,
              pointHitRadius: 30,
              pointBorderWidth: 2,
              pointStyle: 'rectRounded',
            },
          ],
        });
      } catch (error) {
        console.error('Error fetching chart data:', error);
      }
    };
    const interval = setInterval(fetchChartData, 10000);
    return () => clearInterval(interval);

  }, []);

  useEffect(() => {
    const fetchChartDatacp = async () => {
      try {
        const response = await fetch('http://192.168.0.17:8080/tiempohis/cpu');
        const data = await response.json();

        const labels = data.histcpus.map(item => item.fech);
        const dataValues = data.histcpus.map(item => item.histcpu);

        setChartDatahiscp({
          labels,
          datasets: [
            {
              label: 'CPU Usado',
              data: dataValues,
              borderColor: 'orange',
              backgroundColor: 'transparent',
              pointBorderColor: 'orange',
              pointBackgroundColor: 'rgba(255,150,0,0.5)',
              pointRadius: 5,
              pointHoverRadius: 10,
              pointHitRadius: 30,
              pointBorderWidth: 2,
              pointStyle: 'rectRounded',
            },
          ],
        });
      } catch (error) {
        console.error('Error fetching chart datacp:', error);
      }
    };
    const interval = setInterval(fetchChartDatacp, 10000);
    return () => clearInterval(interval);

  }, []);

  const { freeRam, cpuInfo } = data;
  const totalRam = 16000000;
  const useRam = totalRam - freeRam;
  const porcentajeUsado = ((totalRam - freeRam) / totalRam) * 100;
  const cpuUso = cpuInfo?.cpuTotal - cpuInfo?.cpuPorcentaje;

  const cpuLibre = cpuInfo ? 100 - (cpuInfo.cpu_porcentaje / cpuInfo.cpu_total * 100) : 0;
  const cpuEnUso = cpuInfo ? (cpuInfo.cpu_porcentaje / cpuInfo.cpu_total * 100) : 0;
  const chartData = {
    labels: ['Memoria libre', 'Memoria en uso'],
    datasets: [
      {
        label: 'Memoria RAM',
        data: [freeRam, useRam],
        backgroundColor: ['#2ECC71', '#E74C3C'],
        borderColor: ['#2ECC71', '#E74C3C'],
        borderWidth: 1
      }
    ]
  };

  const cpuChartData = {
    labels: ['CPU libre', 'CPU en uso'],
    datasets: [
      {
        label: 'CPU',
        data: [cpuLibre, cpuEnUso],
        backgroundColor: ['#2ECC71', '#E74C3C'],
        borderColor: ['#2ECC71', '#E74C3C'],
        borderWidth: 1
      }
    ]
  };

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
                      <th scope="col">Memoria</th>
                      <th scope="col">Porcentaje Memoria</th>
                    </tr>
                  </thead>
                  <tbody>
                    <tr>
                      <td>RAM</td>
                      <td>{freeRam} MB</td>
                      <td>{useRam} MB</td>
                      <td>{porcentajeUsado.toFixed(2)}%</td>
                    </tr>
                    <tr>
                      <td>CPU</td>
                      <td>{cpuLibre.toFixed(2)}%</td>
                      <td>{cpuEnUso.toFixed(2)}%</td>
                      <td>{(cpuUso ? (cpuUso / cpuInfo.cpuTotal * 100) : 0).toFixed(2)}%</td>
                    </tr>
                  </tbody>
                </table>
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
                      <th scope="col">Tamaño</th>
                      <th scope="col">Fecha</th>
                    </tr>
                  </thead>
                  <tbody>
                    <tr>
                      <td>RAM</td>
                      <td>{freeRam} MB</td>
                      <td>{useRam} MB</td>
                      <td>{porcentajeUsado.toFixed(2)}%</td>
                    </tr>
                    <tr>
                      <td>CPU</td>
                      <td>{cpuLibre.toFixed(2)}%</td>
                      <td>{cpuEnUso.toFixed(2)}%</td>
                      <td>{(cpuUso ? (cpuUso / cpuInfo.cpuTotal * 100) : 0).toFixed(2)}%</td>
                    </tr>
                  </tbody>
                </table>
              </div>
            </div>

          </div>
        </div>
      </section>
    </div>
  );
}

export default App;

