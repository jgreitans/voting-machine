namespace webapp.Locations.Models
{
    public class LocationModel
    {
        public LocationModel() {
        }

        public LocationModel(string id, string name)
        {
            Id = id;
            Name = name;
        }

        public string Id { get; set; }
        public string Name { get; set; }
    }
}
