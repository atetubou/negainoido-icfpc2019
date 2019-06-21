function draw_data(data) {
  var table = new Tabulator("#content", {
    data: [],           //load row data from array
    layout:"fitColumns",      //fit columns to width of table
    responsiveLayout:"hide",  //hide columns that dont fit on the table
    tooltips:true,            //show tool tips on cells
    addRowPos:"top",          //when adding a new row, add it to the top of the table
    history:true,             //allow undo and redo actions on the table
    pagination:"local",       //paginate the data
    paginationSize:7,         //allow 7 rows per page of data
    movableColumns:true,      //allow column order to be changed
    resizableRows:true,       //allow row order to be changed
    initialSort:[             //set the initial sort order of the data
      {column:"name", dir:"asc"},
    ],
    columns:[                 //define the table columns
      {title:"Name", field:"name", editor:"input"},
      {title:"Task Progress", field:"progress", align:"left", formatter:"progress", editor:true},
      {title:"Gender", field:"gender", width:95, editor:"select", editorParams:{values:["male", "female"]}},
      {title:"Rating", field:"rating", formatter:"star", align:"center", width:100, editor:true},
      {title:"Color", field:"col", width:130, editor:"input"},
      {title:"Date Of Birth", field:"dob", width:130, sorter:"date", align:"center"},
      {title:"Driver", field:"car", width:90,  align:"center", formatter:"tickCross", sorter:"boolean", editor:true},
    ],
  });
}

function load_data() {
  var xhr = new XMLHttpRequest();
  xhr.open('GET', '/score/stat/api', false);
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
