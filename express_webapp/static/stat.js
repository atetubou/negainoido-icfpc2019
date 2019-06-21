function draw_data(data) {

  var rows = [];
  for (var i = 0; i < data.values.length; ++i) {
    rows[i] = data.values[i].best;
  }

  var table = new Tabulator("#content", {
    height: 10000,
    data: rows,           //load row data from array
    layout:"fitColumns",      //fit columns to width of table
    responsiveLayout:"hide",  //hide columns that dont fit on the table
    tooltips:true,            //show tool tips on cells
    addRowPos:"top",          //when adding a new row, add it to the top of the table
    history:true,             //allow undo and redo actions on the table
    pagination:"local",       //paginate the data
    paginationSize: 100000,         //allow 7 rows per page of data
    movableColumns:true,      //allow column order to be changed
    resizableRows:true,       //allow row order to be changed
    initialSort:[             //set the initial sort order of the data
      {column:"task_id", dir:"asc"},
    ],
    columns: [
      {title: "best",
        columns: [
          {title:"id", field:"id"},
          {title:"solver", field:"solver"},
          {title:"task_id", field:"task_id"},
          {title:"score", field:"score"},
          {title:"created", field:"created"},
          {title:"updatedAt", field:"updatedAt"},
        ]
      }
    ]
  });
}

function load_data() {
  var xhr = new XMLHttpRequest();
  xhr.open('GET', '/score/stat/api', true);
  xhr.onreadystatechange = function() {
    switch ( xhr.readyState ) {
      case 0:
        break;
      case 1: // データ送信中.
        break;
      case 2: // 応答待ち.
        break;
      case 3: // データ受信中.
        break;
      case 4: // データ受信完了.
        if( xhr.status == 200 || xhr.status == 304 ) {
          var data = JSON.parse(xhr.responseText);
          draw_data(data);
        }
        break;
    }
  };
  xhr.send();
}

setTimeout(load_data, 500);
setInterval(load_data, 20000);
